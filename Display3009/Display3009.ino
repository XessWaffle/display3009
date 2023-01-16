#include <FastLED.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "BladeManager.h" 
#include "BladeFrameIterator.h"

// How many leds in your strip?

#define NUM_LEDS 72
#define NUM_ANIMATIONS 4

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN_FOLLOWER 33
#define CLOCK_PIN_FOLLOWER 32
#define DATA_PIN_PRIMARY 19
#define CLOCK_PIN_PRIMARY 14
#define MOTOR_PIN 13

uint8_t createReadInstruction(uint8_t inst){
  return (uint8_t) (inst | 0x1 << 7);
}

uint8_t createWriteInstruction(uint8_t inst){
  return (uint8_t) (inst & ~(0x1 << 7));
}


#define XESSAMD "192.168.137.71"
#define MESSAGE_PAUSE 0xFF
#define REFRESH 0xFF
#define DISCONNECT 0xFE
#define STREAM 0xFD
#define THROTTLE createWriteInstruction(0x00)
#define START createWriteInstruction(0x01)
#define STOP createWriteInstruction(0x02)
#define TEST createReadInstruction(0x03)
#define DATA createReadInstruction(0x04)
#define STATE createReadInstruction(0x05)
#define ANIMATION createWriteInstruction(0x06)
#define MULTIPLIER createWriteInstruction(0x07)
#define FPS createWriteInstruction(0x08)

// Define the array of leds
struct CRGB primary[NUM_LEDS], follower[NUM_LEDS];

BladeManager bladeManager;
BladeFrameIterator frameIterator[NUM_ANIMATIONS];
BladeFrameIterator *currIterator; 

const char* ssid     = "3009 Xess";
const char* password = "dingd0ngditch";
const int port = 80;
const char* host = XESSAMD;

WiFiClient client, streamClient;

bool streaming = false;

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

void stream(const char* info){
  if(streaming) client.print(info);
}

void onRefresh(){
  client.write((uint8_t) 9);
  writeInstruction("throttle", THROTTLE);
  writeInstruction("start", START);
  writeInstruction("stop", STOP);
  writeInstruction("test", TEST);
  writeInstruction("data", DATA);
  writeInstruction("state", STATE);
  writeInstruction("animation", ANIMATION);
  writeInstruction("multiplier", MULTIPLIER);
  writeInstruction("fps", FPS);
}

void onDisconnect(){
  client.stop();
}

void prepAnimations(){
  bladeManager.DisableTimer();
  frameIterator[0] = BladeFrameIterator();
  BladeFrame *zero = new BladeFrame();
  ArmFrame *pre = new ArmFrame(NUM_LEDS), *black = new ArmFrame(NUM_LEDS);
  for(int i = 50; i < NUM_LEDS; i++){
    pre->SetLED(i, CRGB::Red);
  }
  zero->AddArmFrame(pre, 0.00);
  zero->AddArmFrame(black, 0.01);
  frameIterator[0].AddFrame(zero);

  frameIterator[1] = BladeFrameIterator();
  ArmFrame *orange = new ArmFrame(NUM_LEDS), *red = new ArmFrame(NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i++){
    if(i > 30){
      orange->SetLED(i, CRGB::Orange);
      red->SetLED(i, CRGB::Red);
    }
  }

  for(int i = 0; i < 20; i++){
    BladeFrame *f1 = new BladeFrame();
    if(i != 0)
      f1->AddArmFrame(red, 0.0);
    f1->AddArmFrame(orange, i/20.0 * TWO_PI);
    f1->AddArmFrame(black, (i + 1)/20.0 * TWO_PI);
    frameIterator[1].AddFrame(f1);
  }


  frameIterator[2] = BladeFrameIterator();
  ArmFrame *purple = new ArmFrame(NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i+=3){
    purple->SetLED(i, CRGB::Purple);
  }

  for(int i = 0; i < 20; i++){
    BladeFrame *f1 = new BladeFrame();
    for(int j = 0; j < 3; j++){
      f1->AddArmFrame(purple, j * TWO_PI/3.0);
      f1->AddArmFrame(black, j * TWO_PI/3.0 + i * TWO_PI/60.0 + 0.01);
    }
    frameIterator[2].AddFrame(f1);
  }

  frameIterator[3] = BladeFrameIterator(BladeFrameIterator::REWIND);
  for(int i = 0; i < NUM_LEDS; i++){
    ArmFrame* a1 = new ArmFrame(NUM_LEDS);
    BladeFrame *f1 = new BladeFrame();
    a1->SetLED(i, CRGB::Turquoise);
    f1->AddArmFrame(a1, 0.0);
    frameIterator[3].AddFrame(f1);
  }
  bladeManager.EnableTimer();
}

void setup() {

  Serial.begin(115200);

  FastLED.setMaxRefreshRate(0);
  FastLED.addLeds<SK9822, DATA_PIN_PRIMARY, CLOCK_PIN_PRIMARY, BGR>(primary, NUM_LEDS);  // BGR ordering is typical
  FastLED.addLeds<SK9822, DATA_PIN_FOLLOWER, CLOCK_PIN_FOLLOWER, BGR>(follower, NUM_LEDS);  // BGR ordering is typical
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  connect();

  bladeManager = BladeManager(MOTOR_PIN);
  bladeManager.SetTarget(1250);

  prepAnimations();

  currIterator = &(frameIterator[0]);
  bladeManager.SetTrigger(currIterator->GetFrame(), primary, follower);
}

void loop() {
  if(!client.connected()){
    connect();
  }

  if(client.available()){
    uint8_t byte = client.read();

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
      sprintf(buff, "%.2f %.2f\r", bladeManager.GetAngularPosition(), bladeManager.GetAngularVelocity());
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
    } else if(byte == ANIMATION){
      int animation;
      client.readBytes((char*) &animation, sizeof(int));
      if(animation < 0)
        animation = 0;
      if(animation >= NUM_ANIMATIONS)
        animation = NUM_ANIMATIONS - 1;

      currIterator = &(frameIterator[animation]);
      
    } else if(byte == MULTIPLIER){
      int multiplier;
      client.readBytes((char*) &multiplier, sizeof(int));
      bladeManager.SetDriftMultiplier((double) multiplier * MULTIPLIER_DIVISIONS);
    } else if(byte == FPS){
      int fps;
      client.readBytes((char*) &fps, sizeof(int));
      if(fps <= 0)
        fps = 30;
      currIterator->SetFrameRate(fps);
    } else if(byte == STREAM){
      streaming = !streaming;
    }

  }
  if(bladeManager.Step()) FastLED.show();
  if(currIterator != NULL && currIterator->Step()) bladeManager.SetTrigger(currIterator->GetFrame(), primary, follower);
}
