#include <FastLED.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "BladeManager.h" 
#include "BladeFrameIterator.h"

// How many leds in your strip?

#define NUM_LEDS 72
#define NUM_STRIPS 2
#define NUM_ANIMATIONS 6


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

uint8_t createStreamClient(uint8_t id) {
  return (uint8_t) (id | 0x1 << 7);
}

#define XESSAMD "192.168.137.71"
#define MESSAGE_PAUSE 0xFF
#define REFRESH 0xFF
#define DISCONNECT 0xFE
#define THROTTLE createWriteInstruction(0x00)
#define START createWriteInstruction(0x01)
#define STOP createWriteInstruction(0x02)
#define TEST createReadInstruction(0x03)
#define STATE createReadInstruction(0x05)
#define ANIMATION createWriteInstruction(0x06)
#define FPS createWriteInstruction(0x07)
#define MULTIPLIER createWriteInstruction(0x08)

// Define the array of leds
struct CRGB primary[NUM_LEDS], follower[NUM_LEDS];

struct Blade{
  double omega = 40, theta;
  double multiplier = 1.0;
  double rezero = true;
  BladeFrame *currFrame = NULL;
};

volatile Blade blade;
BladeManager bladeManager;
BladeFrameIterator frameIterator[NUM_ANIMATIONS];
BladeFrameIterator *currIterator; 

const char* ssid     = "3009 Xess";
const char* password = "dingd0ngditch";
const int port = 80;
const char* host = XESSAMD;

WiFiClient client;

TaskHandle_t DAQ_TASK, DISP_TASK;
SemaphoreHandle_t bladeMutex = xSemaphoreCreateMutex();

void connect(){
  if (client.connect(host, port)) {
    client.write((uint8_t) 0x01);
  }
}

void writeInstruction(const char* instStr, uint8_t inst){
  client.print(instStr);
  client.write(MESSAGE_PAUSE);
  client.write(inst);
}

void onRefresh(){
  client.write((uint8_t) 8);
  writeInstruction("throttle", THROTTLE);
  writeInstruction("start", START);
  writeInstruction("stop", STOP);
  writeInstruction("test", TEST);
  writeInstruction("state", STATE);
  writeInstruction("anim", ANIMATION);
  writeInstruction("fps", FPS);
  writeInstruction("mult", MULTIPLIER);
}

void onDisconnect(){
  client.stop();
}

void prepAnimations(){
  frameIterator[0] = BladeFrameIterator();
  BladeFrame *zero = new BladeFrame();
  ArmFrame *pre = new ArmFrame(NUM_LEDS), *black = new ArmFrame(NUM_LEDS);
  for(int i = 50; i < NUM_LEDS; i++){
    pre->SetLED(i, CRGB::Red);
  }
  zero->AddArmFrame(pre, 0.0);
  zero->AddArmFrame(black, PI/2.0);
  zero->AddArmFrame(pre, PI);
  zero->AddArmFrame(black, 3 * PI/2.0);
  frameIterator[0].AddFrame(zero);

  frameIterator[1] = BladeFrameIterator();
  ArmFrame *orange = new ArmFrame(NUM_LEDS), *red = new ArmFrame(NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i++){
    if(i > 50){
      orange->SetLED(i, CRGB::Orange);
      red->SetLED(i, CRGB::Red);
    }
  }

  for(int i = 0; i < 40; i++){
    BladeFrame *f1 = new BladeFrame();
    if(i != 0)
      f1->AddArmFrame(red, 0.0);
    f1->AddArmFrame(orange, i/40.0 * TWO_PI);
    f1->AddArmFrame(black, (i + 1)/40.0 * TWO_PI);
    frameIterator[1].AddFrame(f1);
  }

  frameIterator[2] = BladeFrameIterator();
  ArmFrame *purple = new ArmFrame(NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i+=3){
    purple->SetLED(i, CRGB(255, 0, 255));
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

  frameIterator[4] = BladeFrameIterator(BladeFrameIterator::REWIND);
  ArmFrame *white = new ArmFrame(NUM_LEDS), *redFull = new ArmFrame(NUM_LEDS), *greenFull = new ArmFrame(NUM_LEDS), *blueFull = new ArmFrame(NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i++){
    redFull->SetLED(i, CRGB::Red);
    greenFull->SetLED(i, CRGB(0, 255, 0));
    blueFull->SetLED(i, CRGB::Blue);
    white->SetLED(i, CRGB::White);    
  }

  for(int i = 1; i <= 20; i++){
    BladeFrame *f1 = new BladeFrame();
    
    double div = TWO_PI/i;
    for(int j = 0; j < i; j++){
      f1->AddArmFrame(redFull, j * TWO_PI/i);
      f1->AddArmFrame(greenFull, j * TWO_PI/i + TWO_PI/(3 * i));
      f1->AddArmFrame(blueFull, j * TWO_PI/i + 2 * TWO_PI/(3 * i));
    }
    frameIterator[4].AddFrame(f1);
  }

  BladeFrame *f1 = new BladeFrame();
  f1->AddArmFrame(white, 0.0);
  frameIterator[4].AddFrame(f1);

  frameIterator[5] = BladeFrameIterator();
  ArmFrame *test = new ArmFrame(NUM_LEDS);
  for(int i = 0; i <= NUM_LEDS; i++){
    if(i % 5 == 0)
      test->SetLED(i, CRGB::Gold);
  }

  f1 = new BladeFrame();
  f1->AddArmFrame(pre, 0.0);
  for(int i = 1; i < 100; i++){
    f1->AddArmFrame(i%2 == 0 ? test : black, i * TWO_PI/100);
  }
  frameIterator[5].AddFrame(f1);

}


void setup() {

  Serial.begin(115200);

  FastLED.addLeds<SK9822, DATA_PIN_PRIMARY, CLOCK_PIN_PRIMARY, BGR, DATA_RATE_MHZ(40)>(primary, NUM_LEDS);  // BGR ordering is typical
  FastLED.addLeds<SK9822, DATA_PIN_FOLLOWER, CLOCK_PIN_FOLLOWER, BGR, DATA_RATE_MHZ(40)>(follower, NUM_LEDS);  // BGR ordering is typical
  //FastLED.setMaxRefreshRate(0);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  connect();

  bladeManager = BladeManager(MOTOR_PIN);
  bladeManager.SetTarget(1300);

  prepAnimations();

  currIterator = &(frameIterator[0]);
  blade.currFrame = currIterator->GetFrame();

  xTaskCreatePinnedToCore(
                    DAQ_WRAPPER, /* Task function. */
                    "Data",      /* name of task. */
                    8192,        /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &DAQ_TASK,   /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500);
  xTaskCreatePinnedToCore(
                    DISP_WRAPPER,/* Task function. */
                    "Task2",     /* name of task. */
                    8192,        /* Stack size of task */
                    NULL,        /* parameter of the task */
                    5,           /* priority of the task */
                    &DISP_TASK,  /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500); 
}

void DAQ() {
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
      
      xSemaphoreTake(bladeMutex, portMAX_DELAY);
        
      currIterator = &(frameIterator[animation]);
      blade.currFrame = currIterator->GetFrame();

      xSemaphoreGive(bladeMutex);
    } else if(byte == FPS){
      int fps;
      client.readBytes((char*) &fps, sizeof(int));
      if(fps <= 0)
        fps = 30;
      currIterator->SetFrameRate(fps);
    } else if(byte == MULTIPLIER){
      int mult;
      client.readBytes((char*) &mult, sizeof(int));
      double trueMult = mult/1000.0;
      xSemaphoreTake(bladeMutex, portMAX_DELAY);
      blade.multiplier = trueMult;
      xSemaphoreGive(bladeMutex);
    }
  }
  
  bool reachedZero = bladeManager.Step();
  bool frameChange = currIterator->Step();
  
  xSemaphoreTake(bladeMutex, portMAX_DELAY);

  if(currIterator != NULL && frameChange)
    blade.currFrame = currIterator->GetFrame(); 

  if(reachedZero && blade.rezero){
    blade.theta = 0.0;
    blade.rezero = false;
  }
  
  xSemaphoreGive(bladeMutex);

}

void DISP(long step, bool updatePrimary) {
 
  xSemaphoreTake(bladeMutex, portMAX_DELAY);

  blade.theta += blade.multiplier * blade.omega * step * 0.000001;
  if(blade.theta > TWO_PI) {
    blade.theta -= TWO_PI;
    blade.rezero = true;
  }

  ArmFrame *frame = NULL;
  bool frameUpdated = false;

  if(blade.currFrame != NULL){
    frameUpdated = updatePrimary ? blade.currFrame->UpdatePrimaryFrame(blade.theta) : blade.currFrame->UpdateFollowerFrame(blade.theta);
    frame = updatePrimary ? blade.currFrame->GetPrimaryFrame() : blade.currFrame->GetFollowerFrame();
  }

  xSemaphoreGive(bladeMutex);

  if(frame != NULL && frameUpdated){
    frame->Trigger(updatePrimary ? primary : follower);
    RENDER(updatePrimary);
  }
}

void DAQ_WRAPPER(void *params){
  for(;;) DAQ();
}

void DISP_WRAPPER(void *params){
  long step = 0;
  bool primary = true;
  for(;;) {
    long time = micros();
    DISP(step, primary);
    primary = !primary;
    step = micros() - time;
  }
}


void RENDER(bool refreshPrimary){
  int dataPin = refreshPrimary ? DATA_PIN_PRIMARY : DATA_PIN_FOLLOWER;
  int clockPin = refreshPrimary ? CLOCK_PIN_PRIMARY : CLOCK_PIN_FOLLOWER;
  int regSet = refreshPrimary ? GPIO_OUT_W1TS_REG : GPIO_OUT1_W1TS_REG;
  int regClear = refreshPrimary ? GPIO_OUT_W1TC_REG : GPIO_OUT1_W1TC_REG;

  dataPin = dataPin > 32 ? dataPin - 32 : dataPin;
  dataPin = 1 << dataPin;
  clockPin = clockPin > 32 ? clockPin - 32 : clockPin;
  clockPin = 1 << clockPin;

  REG_WRITE(regClear, clockPin);
  REG_WRITE(regClear, dataPin);

  // Start Frame
  REG_WRITE(regClear, dataPin);
  for(int i = 0; i < 32; i++){
    REG_WRITE(regSet, clockPin);
    REG_WRITE(regClear, clockPin);
  }

  // LED Frames
  for(int i = 0; i < NUM_LEDS; i++) {

    CRGB push = refreshPrimary ? primary[i] : follower[i];

    // LED Frame Brightness
    REG_WRITE(regSet, dataPin);
    for(int j = 0; j < 8; j++){
      REG_WRITE(regSet, clockPin);
      REG_WRITE(regClear, clockPin);
    }

    int j = 1;
    // LED Blue
    while(true){
      if(j >= 1 << 8) break;
      bool set = (push.blue & j) > 0;
      REG_WRITE(set ? regSet : regClear, dataPin);
      REG_WRITE(regSet, clockPin);
      REG_WRITE(regClear, clockPin);
      j = j << 1;
    }
    
    j = 1;
    // LED Green
    while(true){
      if(j >= 1 << 8) break;
      bool set = (push.green & j) > 0;
      REG_WRITE(set ? regSet : regClear, dataPin);
      REG_WRITE(regSet, clockPin);
      REG_WRITE(regClear, clockPin);
      j = j << 1;
    }
    
    j = 1;
    // LED Red 
    while(true){
      if(j >= 1 << 8) break;
      bool set = (push.red & j) > 0;
      REG_WRITE(set ? regSet : regClear, dataPin);
      REG_WRITE(regSet, clockPin);
      REG_WRITE(regClear, clockPin);
      j = j << 1;
    }
  }

  // End Frame
  REG_WRITE(regClear, dataPin);
  for(int i = 0; i < 36; i++){
    REG_WRITE(regSet, clockPin);
    REG_WRITE(regClear, clockPin);
  }
}

void loop(){
  vTaskDelete(NULL);
}
