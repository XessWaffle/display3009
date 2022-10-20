#include <FastLED.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "BladeManager.h" 
#include "BladeFrameIterator.h"

#define ANGULAR_FRAME_NOISE 0.005

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
    Serial.println("connection failed");
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
  client.write((uint8_t) 8);
  writeInstruction("throttle", THROTTLE);
  writeInstruction("start", START);
  writeInstruction("stop", STOP);
  writeInstruction("test", TEST);
  writeInstruction("data", DATA);
  writeInstruction("state", STATE);
  writeInstruction("toggle", TOGGLE);
  writeInstruction("multiplier", MULTIPLIER);
}

void onDisconnect(){
  client.stop();
}


void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, PRIMARY_PIN, GRB>(primary, NUM_LEDS);  // GRB ordering is typical
  FastLED.addLeds<WS2812B, FOLLOWER_PIN, GRB>(follower, NUM_LEDS);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  connect();

  bladeManager = BladeManager(MOTOR_PIN);
  bladeManager.SetTarget(1250);


  ArmFrame* a1 = new ArmFrame(ArmFrame::HOLD, NUM_LEDS);
  ArmFrame* a2 = new ArmFrame(ArmFrame::HOLD, NUM_LEDS);
  ArmFrame* a3 = new ArmFrame(ArmFrame::HOLD, NUM_LEDS);

  Serial.println("Created Arm Frame");

  for(int i = 0; i < NUM_LEDS; i++){
    if(i > 30){
      a1->SetLED(i, CRGB::Red);
      a2->SetLED(i, CRGB::White);
      a3->SetLED(i, CRGB::Blue);
    } else {
      a1->SetLED(i, CRGB::Black);
      a2->SetLED(i, CRGB::Black);
      a3->SetLED(i, CRGB::Black);
    }
  }

  Serial.println("Updated Colors for Arm Frame");
  
  BladeFrame *f1 = new BladeFrame();

  f1->AddArmFrame(a1, 0.0);
  f1->AddArmFrame(a2, TWO_PI/3.0);
  f1->AddArmFrame(a3, 2 * TWO_PI/3.0);

  Serial.println("Created and copied Frames");

  frameIterator = BladeFrameIterator(BladeFrameIterator::LOOP);
  frameIterator.AddFrame(f1);

  Serial.println("Ready");
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
      Serial.print("Recieved Throttle Value: ");
      Serial.println(throttle);
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
    }

  }

  double theta = bladeManager.Step();
  BladeFrame *curr = frameIterator.Step();

  ArmFrame *primaryFrame = curr->GetArmFrame(theta, ANGULAR_FRAME_NOISE);
  theta += PI;
  if(theta >= TWO_PI) theta -= TWO_PI;
  ArmFrame *followerFrame = curr->GetArmFrame(theta, ANGULAR_FRAME_NOISE);

  if(primaryFrame != NULL) 
    primaryFrame->Trigger(primary);

  if(followerFrame != NULL)
    followerFrame->Trigger(follower);


}
