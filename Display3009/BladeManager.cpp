#include "BladeManager.h"
#include "Constants.h"

Servo _motor;

BladeManager::BladeManager(){

}

BladeManager::BladeManager(int motorPin){

  this->_data = (struct BladeData*) malloc(sizeof(struct BladeData));

  this->_motorPin = motorPin;
  this->_motorWriteValue = CDAQ::BLADE_STOP_PWM;

  this->_lastStepped = millis();
  this->_desiredWrite = CDAQ::BLADE_STOP_PWM;

  _motor.attach(motorPin);
  _motor.writeMicroseconds(CDAQ::BLADE_MAX_PWM);
  delay(100);
  _motor.writeMicroseconds(CDAQ::BLADE_STOP_PWM);

  this->_hallFilter = (struct RingNode*) malloc(sizeof(struct RingNode));
  
  this->_hallFilter->data = 0;
  this->_hallFilter->prev = this->_hallFilter;
  this->_hallFilter->next = this->_hallFilter;

  struct RingNode *prev = this->_hallFilter;

  for(int i = 1; i < CDAQ::FILTER_NODES; i++){
    struct RingNode *insert = (struct RingNode*) malloc(sizeof(struct RingNode));
    
    insert->data = 0;
    insert->prev = prev;
    insert->next = prev->next;
    prev->next = insert;
    prev = insert;
  }

}

void BladeManager::SetState(SpinState state){
  this->_state = state;
}

int BladeManager::GetState(){
  return this->_state;
}

void BladeManager::StartBlade(){
  this->_state = SpinState::STARTING;
}

void BladeManager::StopBlade(){
  this->_state = SpinState::STOPPING;
}

// Blade API
void BladeManager::SetTarget(int write){
  this->_desiredWrite = write;
}

BladeData *BladeManager::Step(){

  long currentStep = millis();

  int sensorVal = hallRead();

  this->_hallFilterSum += sensorVal - this->_hallFilter->data;
  this->_hallFilter->data = sensorVal;
  this->_hallFilter = this->_hallFilter->next;

  int avg = this->_hallFilterSum / CDAQ::FILTER_NODES;

  if(!this->_triggerState && avg <= CDAQ::HALL_ENTRY_THRESHOLD){
    long delay = micros() - this->_triggerStart;
    this->_data->omega = (TWO_PI - CDAQ::HALL_SWEEP) / (delay * CVAL::MICROS);
    this->_triggerState = 1;
  } else if(this->_triggerState && avg >= CDAQ::HALL_EXIT_THRESHOLD){
    this->_triggerStart = micros();
    this->_data->trigger = true;
    this->_triggerState = 0;
  }


  if(this->_state == SpinState::STARTING){

    if(this->_motorWriteValue >= CDAQ::BLADE_START_PWM){
      this->_state = SpinState::SPINNING;
    } else if(currentStep - this->_lastStepped > CDAQ::BLADE_UPDATE_DELAY){
      this->_motorWriteValue += 1;
      this->_lastStepped = currentStep;
    }

  } else if(this->_state == SpinState::SPINNING){

    bool desiredValue = this->_desiredWrite == this->_motorWriteValue;

    if(currentStep - this->_lastStepped > CDAQ::BLADE_UPDATE_DELAY && !desiredValue){
      this->_motorWriteValue += this->_motorWriteValue < this->_desiredWrite ? 1 : -1;

      if(this->_motorWriteValue >= CDAQ::BLADE_MAX_PWM)
        this->_motorWriteValue = CDAQ::BLADE_MAX_PWM;
      
      if(this->_motorWriteValue <= CDAQ::BLADE_STOP_PWM)
        this->_motorWriteValue = CDAQ::BLADE_STOP_PWM;

      this->_lastStepped = currentStep;
    }

  } else if(this->_state == SpinState::STOPPING){

    if(this->_motorWriteValue < CDAQ::BLADE_START_PWM){
      this->_state = SpinState::STOPPED;
    } else if(currentStep - this->_lastStepped > CDAQ::BLADE_UPDATE_DELAY){
      this->_motorWriteValue += -1;
      this->_lastStepped = currentStep;
    }

  } else if(this->_state == SpinState::STOPPED){
    this->_motorWriteValue = CDAQ::BLADE_STOP_PWM;
  }

  _motor.writeMicroseconds(this->_motorWriteValue);

  return this->_data;
}

