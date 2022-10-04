#include <FastLED.h>
#include <ESP32Servo.h>
#include "AngleSet.h"
#include "BladeManager.h" 

// How many leds in your strip?
#define NUM_LEDS 82

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 19
#define MOTOR_PIN 13
#define HALL_PIN 32

// Define the array of leds
CRGB leds[NUM_LEDS];

BladeManager bladeManager(MOTOR_PIN, HALL_PIN);

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  bladeManager.SetTargetVelocity(20);
  //bladeManager.StartBlade();
}

void loop() { 
  bladeManager.Step();
}
