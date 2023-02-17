#include <FastLED.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include "Constants.h"
#include "BladeManager.h" 
#include "BladeFrameIterator.h"
#include "BladeFrameCreator.h"
#include "CommunicationHandler.h"

// Define the array of leds
struct CRGB primary[CRENDER::NUM_LEDS], follower[CRENDER::NUM_LEDS];
uint8_t renderBuffer[CRENDER::BUFFER_SIZE];

struct Blade{
  double omega = 40, theta;
  double multiplier = 1.0;
  BladeFrame *currFrame = NULL;
  
  uint8_t brightness = 1;
};

volatile Blade blade;
BladeManager bladeManager;
BladeFrameCreator frameCreator;
BladeFrameIterator frameIterator[CRENDER::NUM_ANIMATIONS];
BladeFrameIterator *currIterator; 

SPIClass *primarySPI, *followerSPI;

TaskHandle_t DAQ_TASK, DISP_TASK;
SemaphoreHandle_t bladeMutex = xSemaphoreCreateMutex();

CommunicationHandler comms = CommunicationHandler();

void prepCommunications(){
  comms.SetHandler(CCOMMS::START, &START, 0);
  comms.SetHandler(CCOMMS::STOP, &STOP, 0);
  comms.SetHandler(CCOMMS::STATE, &STATE, 0);
  comms.SetHandler(CCOMMS::TEST, &TEST, 0);
  comms.SetHandler(CCOMMS::THROTTLE, &THROTTLE, sizeof(int));
  comms.SetHandler(CCOMMS::ANIMATION, &ANIMATION, sizeof(int));
  comms.SetHandler(CCOMMS::FPS, &FPS, sizeof(int));
  comms.SetHandler(CCOMMS::MULTIPLIER, &MULTIPLIER, sizeof(int));
  comms.SetHandler(CCOMMS::STAGE_FRAME, &STAGE_FRAME, sizeof(int));
  comms.SetHandler(CCOMMS::STAGE_ARM, &STAGE_ARM, sizeof(int));
  comms.SetHandler(CCOMMS::SET_LED, &SET_LED, 2 * sizeof(int));
  comms.SetHandler(CCOMMS::COPY_ARM, &COPY_ARM, sizeof(int));
  comms.SetHandler(CCOMMS::COMMIT_ARM, &COMMIT_ARM, 0);
  comms.SetHandler(CCOMMS::COMMIT_FRAME, &COMMIT_FRAME, 0);
}

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

  frameIterator[6] = BladeFrameIterator(BladeFrameIterator::STREAM);
  frameCreator = BladeFrameCreator();

}

void prepRenderingUtils(){

  primarySPI = new SPIClass(HSPI);
  followerSPI = new SPIClass(VSPI);

  pinMode(primarySPI->pinSS(), OUTPUT); // VSPI SS
  pinMode(followerSPI->pinSS(), OUTPUT); // HSPI SS

  primarySPI.begin();
  followerSPI.begin();

  renderBuffer[0] = 0;
  renderBuffer[1] = 0;
  renderBuffer[2] = 0;
  renderBuffer[3] = 0;
  renderBuffer[CRENDER::BUFFER_SIZE - 1] = 0;
  renderBuffer[CRENDER::BUFFER_SIZE - 2] = 0;
  renderBuffer[CRENDER::BUFFER_SIZE - 3] = 0;
  renderBuffer[CRENDER::BUFFER_SIZE - 4] = 0;
  renderBuffer[CRENDER::BUFFER_SIZE - 5] = 0;
}


void setup() {

  Serial.begin(115200);

  //FastLED.addLeds<SK9822, COPS::DATA_PIN_PRIMARY, COPS::CLOCK_PIN_PRIMARY, BGR, DATA_RATE_MHZ(40)>(primary, CRENDER::NUM_LEDS);  // BGR ordering is typical
  //FastLED.addLeds<SK9822, COPS::DATA_PIN_FOLLOWER, COPS::CLOCK_PIN_FOLLOWER, BGR, DATA_RATE_MHZ(40)>(follower, CRENDER::NUM_LEDS);  // BGR ordering is typical
  //FastLED.setMaxRefreshRate(0);

  WiFi.begin(CCOMMS::SSID, CCOMMS::PWD);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }

  prepBuffer();

  prepCommunications();
  
  bladeManager = BladeManager(COPS::MOTOR_PIN);
  bladeManager.SetTarget(1300);

  prepAnimations();

  currIterator = &(frameIterator[0]);
  blade.currFrame = currIterator->GetFrame();

  xTaskCreatePinnedToCore(
                    DAQ_WRAPPER, /* Task function. */
                    "Data",      /* name of task. */
                    16384,        /* Stack size of task */
                    NULL,        /* parameter of the task */
                    2,           /* priority of the task */
                    &DAQ_TASK,   /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */
  xTaskCreatePinnedToCore(
                    DISP_WRAPPER,/* Task function. */
                    "Display",   /* name of task. */
                    8192,        /* Stack size of task */
                    NULL,        /* parameter of the task */
                    5,           /* priority of the task */
                    &DISP_TASK,  /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
}

void DAQ() {
  
  comms.Populate();
  comms.Handle();

  //Serial.println(hallRead());

  bool reachedZero = bladeManager.Step();
  bool frameChange = currIterator->Step();
  
  xSemaphoreTake(bladeMutex, portMAX_DELAY);

  if(currIterator != NULL && frameChange){
    blade.currFrame = currIterator->GetFrame(); 
    blade.brightness = 1;
  }
  
  xSemaphoreGive(bladeMutex);

  vTaskDelay(1);

}

void DISP(long step, bool updatePrimary) {
 
  int brightness = 0;

  xSemaphoreTake(bladeMutex, portMAX_DELAY);

  blade.theta += blade.multiplier * blade.omega * step * 0.000001;
  if(blade.theta > TWO_PI) {
    blade.theta -= TWO_PI;
  }

  if(blade.brighness < 31 && updatePrimary){
    blade.brighness++;
  }

  brightness = blade.brightness;

  ArmFrame *frame = NULL;
  bool frameUpdated = false;

  if(blade.currFrame != NULL){
    frameUpdated = updatePrimary ? blade.currFrame->UpdatePrimaryFrame(blade.theta) : blade.currFrame->UpdateFollowerFrame(blade.theta);
    frame = updatePrimary ? blade.currFrame->GetPrimaryFrame() : blade.currFrame->GetFollowerFrame();
  }

  xSemaphoreGive(bladeMutex);

  if(frame != NULL && frameUpdated){
    frame->Trigger(updatePrimary ? primary : follower);
    RENDER_SPI(updatePrimary, brightness);
    //RENDER_BITBANG(updatePrimary, brightness);
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

void RENDER_SPI(bool refreshPrimary, uint8_t brightness){

  SPIClass* arm = refreshPrimary ? primarySPI : followerSPI;
  CRGB* push = refreshPrimary ? primary : follower;

  for(int i = 0; i < CRENDER::NUM_LEDS; i++) {

    int bufferIndex = 4 * i + 4;

    renderBuffer[bufferIndex] = brightness;
    renderBuffer[bufferIndex + 1] = push[i].blue;
    renderBuffer[bufferIndex + 2] = push[i].green;
    renderBuffer[bufferIndex + 3] = push[i].red;

  }

  arm->writeBytes(renderBuffer, CRENDER::BUFFER_SIZE);

}

void RENDER_BITBANG(bool refreshPrimary, uint8_t brightness){
  int dataPin = refreshPrimary ? COPS::DATA_PIN_PRIMARY : COPS::DATA_PIN_FOLLOWER;
  int clockPin = refreshPrimary ? COPS::CLOCK_PIN_PRIMARY : COPS::CLOCK_PIN_FOLLOWER;
  int regSet = GPIO_OUT_W1TS_REG;
  int regClear = GPIO_OUT_W1TC_REG;

  dataPin = dataPin > 32 ? dataPin - 32 : dataPin;
  dataPin = 1 << dataPin;
  clockPin = clockPin > 32 ? clockPin - 32 : clockPin;
  clockPin = 1 << clockPin;

  REG_WRITE(regClear, clockPin);
  REG_WRITE(regClear, dataPin);

  // Start Frame (4 bytes)
  REG_WRITE(regClear, dataPin);
  for(int i = 0; i < 32; i++){
    REG_WRITE(regSet, clockPin);
    REG_WRITE(regClear, clockPin);
  }

  // LED Frames (72 * 4 bytes)
  for(int i = 0; i < CRENDER::NUM_LEDS; i++) {

    CRGB push = refreshPrimary ? primary[i] : follower[i];

    // LED Frame Brightness
    REG_WRITE(regSet, dataPin);
    for(int j = 0; j < 8; j++){
      if(j >= 3)
        REG_WRITE((1 << (j - 3) & brightness) > 0 ? regSet : regClear, dataPin);
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

  // End Frame (5 bytes)
  REG_WRITE(regClear, dataPin);
  for(int i = 0; i < 36; i++){
    REG_WRITE(regSet, clockPin);
    REG_WRITE(regClear, clockPin);
  }
}

void START(InstructionNode *node){
  bladeManager.StartBlade();
}

void STOP(InstructionNode *node){
  bladeManager.StopBlade();
}

void TEST(InstructionNode *node){
  node->client->print("Ready!\r");
}

void THROTTLE(InstructionNode *node){
  int throttle = node->data[0];
  if(throttle < CDAQ::BLADE_STOP_PWM)
    throttle = CDAQ::BLADE_STOP_PWM;
  
  if(throttle > CDAQ::BLADE_MAX_PWM)
    throttle = CDAQ::BLADE_STOP_PWM;

  bladeManager.SetTarget(throttle);
}

void STATE(InstructionNode *node){
  char buff[15];  
  sprintf(buff, "%d\r", bladeManager.GetState());
  node->client->print(buff);
}

void ANIMATION(InstructionNode *node){
  int animation = node->data[0];
  if(animation < 0)
    animation = 0;
  if(animation >= CRENDER::NUM_ANIMATIONS)
    animation = CRENDER::NUM_ANIMATIONS - 1;
  
  xSemaphoreTake(bladeMutex, portMAX_DELAY);
    
  currIterator = &(frameIterator[animation]);
  blade.currFrame = currIterator->GetFrame();

  xSemaphoreGive(bladeMutex);
}

void FPS(InstructionNode *node){
  int fps = node->data[0];
  if(fps <= 0)
    fps = 30;
  currIterator->SetFrameRate(fps);
}

void MULTIPLIER(InstructionNode *node){
  int mult = node->data[0];
  double trueMult = mult/1000.0;
  xSemaphoreTake(bladeMutex, portMAX_DELAY);
  blade.multiplier = trueMult;
  xSemaphoreGive(bladeMutex);
}

void STAGE_FRAME(InstructionNode *node){
  int sectors = node->data[0];
  frameCreator.StageFrame(sectors);
}

void STAGE_ARM(InstructionNode *node){
  int sector = node->data[0];
  frameCreator.StageArm(sector);
}

void COPY_ARM(InstructionNode *node){
  int sector = node->data[0];
  frameCreator.CopyArm(sector);
}

void SET_LED(InstructionNode *node){
  int led = node->data[0], color = node->data[1];
  memcpy(&led, node->buff, sizeof(int));
  memcpy(&color, node->buff + sizeof(int), sizeof(int));
  frameCreator.SetLED(led, CRGB(color));
}

void COMMIT_ARM(InstructionNode *node){
  frameCreator.CommitArm();
  node->client->print("1\r");
}

void COMMIT_FRAME(InstructionNode *node){
  xSemaphoreTake(bladeMutex, portMAX_DELAY);

  BladeFrame *committed = frameCreator.CommitFrame();
  frameIterator[CRENDER::NUM_ANIMATIONS - 1].AddFrame(committed);

  xSemaphoreGive(bladeMutex);  
  
  node->client->print("1\r");
}

void loop(){
  vTaskDelete(NULL);
}
