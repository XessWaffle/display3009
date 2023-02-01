#include <FastLED.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "BladeManager.h" 
#include "BladeFrameIterator.h"
#include "Constants.h"
#include "CommunicationHandler.h"

// Define the array of leds
struct CRGB primary[CRENDER::NUM_LEDS], follower[CRENDER::NUM_LEDS];

struct Blade{
  double omega = 40, theta;
  double multiplier = 1.0;
  double rezero = true;
  BladeFrame *currFrame = NULL;
};

volatile Blade blade;
BladeManager bladeManager;
BladeFrameIterator frameIterator[CRENDER::NUM_ANIMATIONS];
BladeFrameIterator *currIterator; 

TaskHandle_t DAQ_TASK, DISP_TASK;
SemaphoreHandle_t bladeMutex = xSemaphoreCreateMutex();

CommunicationHandler comms;

void prepAnimations(){
  frameIterator[0] = BladeFrameIterator();
  BladeFrame *zero = new BladeFrame();
  ArmFrame *pre = new ArmFrame(CRENDER::NUM_LEDS), *black = new ArmFrame(CRENDER::NUM_LEDS);
  for(int i = 50; i < CRENDER::NUM_LEDS; i++){
    pre->SetLED(i, CRGB::Red);
  }
  zero->AddArmFrame(pre, 0.0);
  zero->AddArmFrame(black, PI/2.0);
  zero->AddArmFrame(pre, PI);
  zero->AddArmFrame(black, 3 * PI/2.0);
  frameIterator[0].AddFrame(zero);

  frameIterator[1] = BladeFrameIterator();
  ArmFrame *orange = new ArmFrame(CRENDER::NUM_LEDS), *red = new ArmFrame(CRENDER::NUM_LEDS);
  for(int i = 0; i < CRENDER::NUM_LEDS; i++){
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
  ArmFrame *purple = new ArmFrame(CRENDER::NUM_LEDS);
  for(int i = 0; i < CRENDER::NUM_LEDS; i+=3){
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
  for(int i = 0; i < CRENDER::NUM_LEDS; i++){
    ArmFrame* a1 = new ArmFrame(CRENDER::NUM_LEDS);
    BladeFrame *f1 = new BladeFrame();
    a1->SetLED(i, CRGB::Turquoise);
    f1->AddArmFrame(a1, 0.0);
    frameIterator[3].AddFrame(f1);
  }

  frameIterator[4] = BladeFrameIterator(BladeFrameIterator::REWIND);
  ArmFrame *white = new ArmFrame(CRENDER::NUM_LEDS), *redFull = new ArmFrame(CRENDER::NUM_LEDS), *greenFull = new ArmFrame(CRENDER::NUM_LEDS), *blueFull = new ArmFrame(CRENDER::NUM_LEDS);
  for(int i = 0; i < CRENDER::NUM_LEDS; i++){
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
  ArmFrame *test = new ArmFrame(CRENDER::NUM_LEDS);
  for(int i = 0; i <= CRENDER::NUM_LEDS; i++){
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

  FastLED.addLeds<SK9822, COPS::DATA_PIN_PRIMARY, COPS::CLOCK_PIN_PRIMARY, BGR, DATA_RATE_MHZ(40)>(primary, CRENDER::NUM_LEDS);  // BGR ordering is typical
  FastLED.addLeds<SK9822, COPS::DATA_PIN_FOLLOWER, COPS::CLOCK_PIN_FOLLOWER, BGR, DATA_RATE_MHZ(40)>(follower, CRENDER::NUM_LEDS);  // BGR ordering is typical
  //FastLED.setMaxRefreshRate(0);

  WiFi.begin(CCOMMS::SSID, CCOMMS::PWD);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  comms = CommunicationHandler();
  comms.SetHandler(CCOMMS::START, &START);
  comms.SetHandler(CCOMMS::STOP, &STOP);
  comms.SetHandler(CCOMMS::STATE, &STATE);
  comms.SetHandler(CCOMMS::TEST, &TEST);
  comms.SetHandler(CCOMMS::THROTTLE, &THROTTLE);
  comms.SetHandler(CCOMMS::ANIMATION, &ANIMATION);
  comms.SetHandler(CCOMMS::FPS, &FPS);
  comms.SetHandler(CCOMMS::MULTIPLIER, &MULTIPLIER);

  bladeManager = BladeManager(COPS::MOTOR_PIN);
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
  xTaskCreatePinnedToCore(
                    DISP_WRAPPER,/* Task function. */
                    "Task2",     /* name of task. */
                    8192,        /* Stack size of task */
                    NULL,        /* parameter of the task */
                    5,           /* priority of the task */
                    &DISP_TASK,  /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
}

void DAQ() {
  
  comms.Step();

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
  int dataPin = refreshPrimary ? COPS::DATA_PIN_PRIMARY : COPS::DATA_PIN_FOLLOWER;
  int clockPin = refreshPrimary ? COPS::CLOCK_PIN_PRIMARY : COPS::CLOCK_PIN_FOLLOWER;
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
  for(int i = 0; i < CRENDER::NUM_LEDS; i++) {

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

void START(WiFiClient client){
  bladeManager.StartBlade();
}

void STOP(WiFiClient client){
  bladeManager.StopBlade();
}

void TEST(WiFiClient client){
  client.print("Ready!\r");
}

void THROTTLE(WiFiClient client){
  int throttle;
  client.readBytes((char*) &throttle, sizeof(int));
  if(throttle < CDAQ::BLADE_STOP_PWM)
    throttle = CDAQ::BLADE_STOP_PWM;
  
  if(throttle > CDAQ::BLADE_MAX_PWM)
    throttle = CDAQ::BLADE_STOP_PWM;

  bladeManager.SetTarget(throttle);
}

void STATE(WiFiClient client){
  char buff[15];  
  sprintf(buff, "%d\r", bladeManager.GetState());
  client.print(buff);
}

void ANIMATION(WiFiClient client){
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
}

void FPS(WiFiClient client){
  int fps;
  client.readBytes((char*) &fps, sizeof(int));
  if(fps <= 0)
    fps = 30;
  currIterator->SetFrameRate(fps);
}

void MULTIPLIER(WiFiClient client){
  int mult;
  client.readBytes((char*) &mult, sizeof(int));
  double trueMult = mult/1000.0;
  xSemaphoreTake(bladeMutex, portMAX_DELAY);
  blade.multiplier = trueMult;
  xSemaphoreGive(bladeMutex);
}

void loop(){
  vTaskDelete(NULL);
}
