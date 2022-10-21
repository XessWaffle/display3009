#include <FastLED.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "BladeManager.h" 
#include "BladeFrameIterator.h"

// How many leds in your strip?
#define NUM_LEDS 41

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define PRIMARY_PIN 19
#define MOTOR_PIN 13
#define FOLLOWER_PIN 32

uint8_t createReadInstruction(uint8_t inst){
  return (uint8_t) (inst | 0x1 << 7);
}

uint8_t createWriteInstruction(uint8_t inst){
  return (uint8_t) (inst & ~(0x1 << 7));
}


#define XESSAMD "192.168.137.7"
#define MESSAGE_PAUSE 0xFF
#define REFRESH 0xFF
#define DISCONNECT 0xFE
#define THROTTLE createWriteInstruction(0x00)
#define START createWriteInstruction(0x01)
#define STOP createWriteInstruction(0x02)
#define TEST createReadInstruction(0x03)
#define DATA createReadInstruction(0x04)
#define STATE createReadInstruction(0x05)
#define TOGGLE createWriteInstruction(0x06)
#define MULTIPLIER createWriteInstruction(0x07)
#define FPS createWriteInstruction(0x08)

// Define the array of leds
struct CRGB primary[NUM_LEDS], follower[NUM_LEDS];

BladeManager bladeManager;
BladeFrameIterator frameIterator;

const char* ssid     = "3009 Xess";
const char* password = "dingd0ngditch";
const int port = 80;
const char* host = XESSAMD;

WiFiClient client;

bool on = false, toggled = false;

void connect(){
  if (!client.connect(host, port)) {

  } else {
    client.write((uint8_t) 0x11);
  }
}

void writeInstruction(const char* instStr, uint8_t inst){
  client.print(instStr);
  client.write(MESSAGE_PAUSE);
  client.write(inst);
}

void onRefresh(){
  Serial.println("Refresh Requested");
  client.write((uint8_t) 9);
  writeInstruction("throttle", THROTTLE);
  writeInstruction("start", START);
  writeInstruction("stop", STOP);
  writeInstruction("test", TEST);
  writeInstruction("data", DATA);
  writeInstruction("state", STATE);
  writeInstruction("toggle", TOGGLE);
  writeInstruction("multiplier", MULTIPLIER);
  writeInstruction("fps", FPS);
}

void onDisconnect(){
  client.stop();
}


void setup() {

  FastLED.addLeds<WS2812B, PRIMARY_PIN, GRB>(primary, NUM_LEDS);  // GRB ordering is typical
  FastLED.addLeds<WS2812B, FOLLOWER_PIN, GRB>(follower, NUM_LEDS);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  connect();

  bladeManager = BladeManager(MOTOR_PIN);
  bladeManager.SetTarget(1250);

  frameIterator = BladeFrameIterator(BladeFrameIterator::REWIND);

  for(int i = 0; i < NUM_LEDS; i++){
    ArmFrame* a1 = new ArmFrame(ArmFrame::HOLD, NUM_LEDS);
    BladeFrame *f1 = new BladeFrame();
    a1->SetLED(i, CRGB::Turquoise);
    f1->AddArmFrame(a1, 0.0);
    frameIterator.AddFrame(f1);
  }

  bladeManager.SetTrigger(frameIterator.GetFrame(), primary, follower);
}

void loop() {

  if(!client.connected()){
    connect();
  }

  if(client.available()){
    uint8_t byte = client.read();
    Serial.println(byte);

    if(byte == REFRESH){
      onRefresh();
    } else if(byte == DISCONNECT){
      onDisconnect();
    } else if(byte == START){
      bladeManager.StartBlade();
    } else if(byte == STOP){
      bladeManager.StopBlade();
    } else if(byte == TEST){
      client.print("Ready!\r");
    } else if(byte == DATA){
      char buff[30];
      sprintf(buff, "%.2f %.2f %.2f %.2f\r", bladeManager.GetAngularPosition(), bladeManager.GetAngularVelocity(), bladeManager.GetLowAngularVelocity(), bladeManager.GetHighAngularVelocity());
      client.print(buff);
    } else if(byte == THROTTLE){
      int throttle;
      client.readBytes((char*) &throttle, sizeof(int));
      if(throttle < BLADE_STOP_PWM)
        throttle = BLADE_STOP_PWM;
      
      if(throttle > BLADE_MAX_PWM)
        throttle = BLADE_STOP_PWM;

      bladeManager.SetTarget(throttle);
    } else if(byte == STATE){
      char buff[15];
      sprintf(buff, "%d\r", bladeManager.GetState());
      client.print(buff);
    } else if(byte == TOGGLE){
      on = !on;
    } else if(byte == MULTIPLIER){
      int multiplier;
      client.readBytes((char*) &multiplier, sizeof(int));
      bladeManager.SetDriftMultiplier((double) multiplier * MULTIPLIER_DIVISIONS);
    } else if(byte == FPS){
      int fps;
      client.readBytes((char*) &fps, sizeof(int));
      if(fps <= 0)
        fps = 30;
      frameIterator.SetFrameRate(fps);
    }

  }

  bladeManager.Step();
  if(frameIterator.Step()) bladeManager.SetTrigger(frameIterator.GetFrame(), primary, follower);
  if(bladeManager.IsTriggered()) FastLED.show();

}
