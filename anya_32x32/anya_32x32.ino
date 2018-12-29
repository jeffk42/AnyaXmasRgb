#include <gamma.h>
#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include <Fonts/FreeSerifItalic12pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

// Most of the signal pins are configurable, but the CLK pin has some
// special constraints.  On 8-bit AVR boards it must be on PORTB...
// Pin 8 works on the Arduino Uno & compatibles (e.g. Adafruit Metro),
// Pin 11 works on the Arduino Mega.  On 32-bit SAMD boards it must be
// on the same PORT as the RGB data pins (D2-D7)...
// Pin 8 works on the Adafruit Metro M0 or Arduino Zero,
// Pin A4 works on the Adafruit Metro M4 (if using the Adafruit RGB
// Matrix Shield, cut trace between CLK pads and run a wire to A4).

// FeatherWing pinouts for M0 and M4
#define CLK  13
#define OE   1  // TX
#define LAT  0  // RX
#define A   A5
#define B   A4
#define C   A3
#define D   A2
#define buttonPin SDA


// the RGB data pins on featherwing, must be on same PORT as CLK
uint8_t rgbpins[] = { 6,5,9,11,10,12 };
 
// Create a 32-pixel tall matrix with the defined pins
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, true, 32, rgbpins);

// The two lines for the right-to-left scrolling marquee
static const char PROGMEM scrollTextLine1[] = {"Anya's"};
static const char PROGMEM scrollTextLine2[] = {"Room"};
// Number of pixels to offset the second line to make it look centered-ish
static const int8_t PROGMEM line2HorizontalOffset = 12;
// The text for the random flashing text page
static const char PROGMEM flashingAroundText[] = {"Anya"};
// The text that's overlayed on the plasma animation.
static const char PROGMEM plasmaText[] = {"ANYA"};

// sine table for the crazy plasma thingy
static const int8_t PROGMEM sinetab[256] = {
     0,   2,   5,   8,  11,  15,  18,  21,
    24,  27,  30,  33,  36,  39,  42,  45,
    48,  51,  54,  56,  59,  62,  65,  67,
    70,  72,  75,  77,  80,  82,  85,  87,
    89,  91,  93,  96,  98, 100, 101, 103,
   105, 107, 108, 110, 111, 113, 114, 116,
   117, 118, 119, 120, 121, 122, 123, 123,
   124, 125, 125, 126, 126, 126, 126, 126,
   127, 126, 126, 126, 126, 126, 125, 125,
   124, 123, 123, 122, 121, 120, 119, 118,
   117, 116, 114, 113, 111, 110, 108, 107,
   105, 103, 101, 100,  98,  96,  93,  91,
    89,  87,  85,  82,  80,  77,  75,  72,
    70,  67,  65,  62,  59,  56,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27,
    24,  21,  18,  15,  11,   8,   5,   2,
     0,  -3,  -6,  -9, -12, -16, -19, -22,
   -25, -28, -31, -34, -37, -40, -43, -46,
   -49, -52, -55, -57, -60, -63, -66, -68,
   -71, -73, -76, -78, -81, -83, -86, -88,
   -90, -92, -94, -97, -99,-101,-102,-104,
  -106,-108,-109,-111,-112,-114,-115,-117,
  -118,-119,-120,-121,-122,-123,-124,-124,
  -125,-126,-126,-127,-127,-127,-127,-127,
  -128,-127,-127,-127,-127,-127,-126,-126,
  -125,-124,-124,-123,-122,-121,-120,-119,
  -118,-117,-115,-114,-112,-111,-109,-108,
  -106,-104,-102,-101, -99, -97, -94, -92,
   -90, -88, -86, -83, -81, -78, -76, -73,
   -71, -68, -66, -63, -60, -57, -55, -52,
   -49, -46, -43, -40, -37, -34, -31, -28,
   -25, -22, -19, -16, -12,  -9,  -6,  -3
};

void setup() {
  matrix.begin();
  pinMode(buttonPin, INPUT_PULLUP);
  // Attach an interrupt to the ISR vector
  attachInterrupt(digitalPinToInterrupt(20), button_down, LOW); // sda 20
}

const float radius1 =16.3, radius2 =23.0, radius3 =40.8, radius4 =44.2,
            centerx1=16.1, centerx2=11.6, centerx3=23.4, centerx4= 4.1,
            centery1= 8.7, centery2= 6.5, centery3=14.0, centery4=-2.9;
float       angle1  = 0.0, angle2  = 0.0, angle3  = 0.0, angle4  = 0.0;
long        hueShift= 0;
bool getSticky = false;
bool buttonInAction = false;
int currentState = 0;

#define FPS 30         // Maximum frames-per-second
uint32_t prevTime = 0; // For frame-to-frame interval timing

void button_down() {
  buttonInAction = true;
}

void marquee() {
    int startPos = 33;
    while (startPos > -75) {
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      matrix.setCursor(startPos, 0);    // start at top left, with one pixel of spacing
      matrix.setTextSize(2);     // size 1 == 8 pixels high
      matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
    
      matrix.setTextColor(matrix.Color333(3, 0, 3));
      matrix.println(scrollTextLine1);
      matrix.setCursor(startPos + line2HorizontalOffset, 17);    // start at top left, with one pixel of spacing
      matrix.println(scrollTextLine2);
      startPos--;
      matrix.swapBuffers(false);
      delay(50);     
    }
}

void randomFlash() {
    int hue = 0;
    for (int flashloop = 0; flashloop < 40; flashloop++) {
      int newX = rand() % 11;
      int newY = rand() % 24;
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      matrix.setCursor(newX, newY);    // start at top left, with one pixel of spacing
      matrix.setTextSize(1);     // size 1 == 8 pixels high
      matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
    
      matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
      hue += 50;
      if(hue >= 1536) hue -= 1536;
      matrix.println(flashingAroundText);
      matrix.swapBuffers(false);
      delay(200);
    }
}

void marqueeRainbow() {
  int startPos = 33;
  int hue = 0;
  while (startPos > -75) {
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    matrix.setCursor(startPos, 0);    // start at top left, with one pixel of spacing
    matrix.setTextSize(2);     // size 1 == 8 pixels high
    matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
  
    matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
    hue += 20;
    if(hue >= 1536) hue -= 1536;
    matrix.println(scrollTextLine1);
    matrix.setCursor(startPos + line2HorizontalOffset, 17);    // start at top left, with one pixel of spacing
    matrix.println(scrollTextLine2);
    startPos--;
    matrix.swapBuffers(false);
    delay(50);
  }
}

void bigLetter() {
  matrix.setCursor(0,23);
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setTextColor(matrix.Color333(0, 0, 7));
  matrix.setFont(&FreeSerifItalic12pt7b);
  matrix.setTextSize(2);
  matrix.print("A");
  matrix.setFont();
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(0, 7, 7));
  matrix.setCursor(27,5);
  matrix.print("N");
  matrix.setCursor(27,13);
  matrix.print("Y");
  matrix.setCursor(27,21);
  matrix.print("A");
  matrix.swapBuffers(false);
  delay(4000);
}

void plasmaName() {
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.swapBuffers(false);
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.swapBuffers(false);
  matrix.setTextColor(matrix.Color333(7, 7, 7));
  matrix.setTextSize(1);
  for (int plasmaloop = 0; plasmaloop < 100; plasmaloop++) {
    int           x1, x2, x3, x4, y1, y2, y3, y4, sx1, sx2, sx3, sx4;
    unsigned char x, y;
    long          value;
  
    // To ensure that animation speed is similar on AVR & SAMD boards,
    // limit frame rate to FPS value (might go slower, but never faster).
    // This is preferable to delay() because the AVR is already plenty slow.
    uint32_t t;
    while(((t = millis()) - prevTime) < (1000 / FPS));
    prevTime = t;
  
    sx1 = (int)(cos(angle1) * radius1 + centerx1);
    sx2 = (int)(cos(angle2) * radius2 + centerx2);
    sx3 = (int)(cos(angle3) * radius3 + centerx3);
    sx4 = (int)(cos(angle4) * radius4 + centerx4);
    y1  = (int)(sin(angle1) * radius1 + centery1);
    y2  = (int)(sin(angle2) * radius2 + centery2);
    y3  = (int)(sin(angle3) * radius3 + centery3);
    y4  = (int)(sin(angle4) * radius4 + centery4);
  
    for(y=0; y<matrix.height(); y++) {
      x1 = sx1; x2 = sx2; x3 = sx3; x4 = sx4;
      for(x=0; x<matrix.width(); x++) {
        value = hueShift
          + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x1 * x1 + y1 * y1) >> 2))
          + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x2 * x2 + y2 * y2) >> 2))
          + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x3 * x3 + y3 * y3) >> 3))
          + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x4 * x4 + y4 * y4) >> 3));
        matrix.drawPixel(x, y, matrix.ColorHSV(value * 3, 255, 200, true));
        x1--; x2--; x3--; x4--;
      }
      y1--; y2--; y3--; y4--;

      matrix.setTextColor(matrix.Color333(7, 7, 7));
      matrix.setCursor(5,12);
      matrix.print(plasmaText);
      matrix.swapBuffers(false);
    }
  
    angle1 += 0.03;
    angle2 -= 0.07;
    angle3 += 0.13;
    angle4 -= 0.15;
    hueShift += 2;
  }
}
void loop() {
  doPhase();
  
  if (buttonInAction) {
    Serial.println("mode switch.");
    getSticky = !getSticky;
    buttonInAction = false;
  }

  if (!getSticky) {
    currentState++;
    if (currentState > 4)
      currentState = 0;
  }
  
}

void doPhase() {
  switch (currentState) {
    case 0:
      marquee();
      break;
    case 1:
      randomFlash();
      break;
    case 2:
      bigLetter();
      break;
    case 3:
      marqueeRainbow();
      break;
    case 4:
      plasmaName();
      break;
    default:
      break;
  }
}
