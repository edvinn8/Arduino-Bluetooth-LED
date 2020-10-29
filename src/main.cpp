#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

void initStrip();
void checkForNewCommands();
void resetStrip();
void colorWipe();
void rainbow(byte wait);
void rainbowCycle(byte wait);
void theaterChase(uint32_t c, byte wait);
void theaterChaseRainbow(uint32_t wait);
void applySelectedMode();
unsigned int parsePrefixedInts(char* receivedChars, const char* prefix);
uint32_t Wheel(byte WheelPos);

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN 6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 45

// Operation Modes
#define COLORWIPE 1
#define RAINBOW 2

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRBW);

const byte numChars = 8;
boolean newData = false;
char receivedChars[numChars];   // an array to store the received data
uint32_t selectedColor = strip.Color(255, 2, 2);


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time LED was updated
byte lastLitPixel = 0;

// Init data
byte brightness = 92;
byte selectedMode = COLORWIPE;
unsigned int interval = 40;

void setup() {
  initStrip();
  Serial.begin(9600);
  Serial.println("<Arduino is ready>");
}

void loop() {
  checkForNewCommands();
  applySelectedMode();
}

void initStrip() {
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();
}

void resetStrip() {
  strip.clear();
  strip.show();
  lastLitPixel = 0;
}

void applySelectedMode() {
  switch (selectedMode)
    {
      case COLORWIPE:
        colorWipe();
        break;
      case RAINBOW: 
        rainbowCycle(interval);
        break;
      default:
        resetStrip();
        break;
    }
}

void checkForNewCommands() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }

  if (newData) {
    Serial.print("This just in ... ");
    Serial.println(receivedChars);

    switch (receivedChars[0]) {
      case 'G': {
        // resetStrip();
        selectedColor = strip.Color(0, 255, 0);
        break;
      }
      case 'R': {
        // resetStrip();
        selectedColor = strip.Color(255, 0, 0);
        break;
      }
      case 'B': {
        // resetStrip();
        selectedColor = strip.Color(0, 0, 255);
        break;
      }
      case 'Y': {
        // resetStrip();
        selectedColor = strip.Color(255, 255, 0);
        break;
      }
      case 'i': {
        char prefix[] = "i";
        interval = parsePrefixedInts(receivedChars, prefix);
        break;
      }
      case 'm': {
        char prefix[] = "m";
        resetStrip();
        selectedMode = parsePrefixedInts(receivedChars, prefix);
        break;
      }
      case 'b': {
        char prefix[] = "b";
        if (brightness == 0) {
          resetStrip();
        }
        brightness = parsePrefixedInts(receivedChars, prefix);
        strip.setBrightness(brightness);
        strip.show();
        break;
      }
      default: {
        selectedColor = strip.Color(0, 0, 0, 255); // White
      }
    }
    newData = false;
  }
}

unsigned int parsePrefixedInts(char* receivedChars, const char * prefix) {
  char * strtokIndx = strtok(receivedChars, prefix);
  return atoi(strtokIndx); 
}

// LED related functions

// Fill the dots one after the other with a color
void colorWipe() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    strip.setPixelColor(lastLitPixel, selectedColor);
    strip.show();
    lastLitPixel++;
    if (lastLitPixel == strip.numPixels()) {
      // reached the end, reset strip
      resetStrip();
    }
  }
}

void rainbow(byte wait) {
  for (uint16_t j = 0; j < 256; j++) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(byte wait) {
  for (uint16_t j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, byte wait) {
  for (uint16_t j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (uint16_t q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint32_t wait) {
  for (uint16_t j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (uint16_t q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
