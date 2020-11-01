#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

void initStrip();
void checkForNewCommands();
void resetStrip();
void colorWipe();
void rainbow(byte wait);
void rainbowCycle();
void theaterChase(uint32_t c, byte wait);
void theaterChaseRainbow(uint32_t wait);
void applySelectedMode();
uint32_t Wheel(byte WheelPos);
uint32_t color_wheel(uint8_t pos);

unsigned int hexToDec(String hexString);
void getArgsof(String message);

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN 6

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 75

// Operation Modes
// #define COLORWIPE 1
// #define RAINBOW 2


SoftwareSerial BTserial(3, 2); // RX | TX
// HC05 TX vers Arduino pin A4 RX.
// HC05 RX vers Arduino pin A3 TX avec diviseur de tension.
// HC05 STATE pin vers Arduino pin A5.

// BTconnected = false lorsque pas connecte
boolean BTconnected = false;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRBW);

boolean newData = false;
uint32_t selectedColor = strip.Color(255, 2, 2);


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time LED was updated
byte lastLitPixel = 0;




enum MODE { SET_MODE, HOLD, AUTO, COLORWIPE, OFF, TV, CUSTOM, SETCOLOR, SETSPEED, BRIGHTNESS, WIPE, RAINBOW, RAINBOWCYCLE, THEATERCHASE, TWINKLERANDOM, THEATERCHASERAINBOW};
bool exit_func = false;      // Global helper variable to get out of the color modes when mode changes

// Init data
int brightness = 92;
// unsigned int interval = 40;
byte selectedMode = COLORWIPE;

// ***************************************************************************
// DEFINE: Default properties
// ***************************************************************************
unsigned int ws2812fx_speed = 196;   // Global variable for storing the delay between color changes --> smaller == faster


// ***************************************************************************
// DEFINE: Datatype LED STATE
// ***************************************************************************
struct ledstate             // Data structure to store a state of a single led
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
};
typedef struct ledstate LEDState;     // Define the datatype LEDState


// ***************************************************************************
// DEFINE: LED STATE for entire LED-Strip
// *************************************************************************** 
LEDState ledstates[LED_COUNT];          // Get an array of led states to store the state of the whole strip
LEDState main_color = { 0, 255, 0, 0};  // Store the "main color" of the strip used in single color modes


void setup() {
  initStrip();
  Serial.begin(9600);
  Serial.println("<Arduino is ready>");
  //comm with HC05
  BTserial.begin(9600);
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
        rainbowCycle();
        break;
      default:
        resetStrip();
        selectedMode = COLORWIPE;
        break;
    }
}

void checkForNewCommands() {
  String received = "";
  while (BTserial.available() > 0 && newData == false) {
    received = BTserial.readString();
    newData = true;
  }

  if (newData) {
    getArgsof(received);

    // apply new data
    selectedColor = strip.Color(main_color.red, main_color.green, main_color.blue, main_color.white);
    strip.setBrightness(brightness);
    newData = false;
  }
}


unsigned int hexToDec(String hexString) {
    unsigned int decValue = 0;
    int nextInt;
    for (unsigned int i = 0; i < hexString.length(); i++) {
        nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
        nextInt = constrain(nextInt, 0, 15);
        decValue = (decValue * 16) + nextInt;
      }
    return decValue;
}




void getArgsof(String message){

             // validate form, massage is in format [00:0000FA00:3D:31]
            if (!(message.length() == 19 && (message.startsWith("["))&&(message.endsWith("]")))){
                  Serial.println("[ERROR]: Form Validation failed!");
                  // gradually slow down animations, to allow message parsing
                  ws2812fx_speed += 100;
                  // notify the app that the last command wasn't applied and ask for a resend
                  BTserial.write("X");
                  return;
            }
            Serial.println(message);
            
            String msg = message;
            // remove Frame
            msg = msg.substring(1,message.indexOf("]"));

                  int  lmode  =  hexToDec(msg.substring(0,2));  
                  selectedMode = lmode;
                  String wrgb = msg.substring(3,11);
                
                  uint8_t w  = hexToDec(wrgb.substring(0,2));      main_color.white   = w;
                  uint8_t r  = hexToDec(wrgb.substring(2,4));      main_color.red     = r;
                  uint8_t g  = hexToDec(wrgb.substring(4,6));      main_color.green   = g;
                  uint8_t b  = hexToDec(wrgb.substring(6));        main_color.blue    = b;
                
                  int br  = hexToDec(msg.substring(12,14));    brightness         = br;
                  int lspeed  = hexToDec(msg.substring(15)); 
                  
                  ws2812fx_speed = constrain(lspeed, 1, 255);
                  main_color.white = constrain(main_color.white, 0, 255);
                  main_color.red   = constrain(main_color.red,   0, 255);
                  main_color.green = constrain(main_color.green, 0, 255);
                  main_color.blue  = constrain(main_color.blue,  0, 255);
            
              Serial.print("Mode: ");
              Serial.print(selectedMode);
              Serial.print(", Color: ");
              Serial.print(main_color.white);
              Serial.print(", ");
              Serial.print(main_color.red);
              Serial.print(", ");
              Serial.print(main_color.green);
              Serial.print(", ");
              Serial.print(main_color.blue);
              Serial.print(", Speed:");
              Serial.print(ws2812fx_speed);
              Serial.print(", Brightness:");
              Serial.println(brightness);
            }


/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t color_wheel(uint8_t pos) {
  pos = 255 - pos;
  if(pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if(pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}


// LED related functions

// Fill the dots one after the other with a color
void colorWipe() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= ws2812fx_speed) {
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
void rainbowCycle() {
  for (uint16_t j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(ws2812fx_speed);
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
