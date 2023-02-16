#include "BladeManager.h"
#include "Constants.h"

Servo _motor;

BladeManager::BladeManager(){

}

BladeManager::BladeManager(int motorPin){

  this->_motorPin = motorPin;
  this->_motorWriteValue = CDAQ::BLADE_STOP_PWM;

  this->_lastStepped = millis();
  this->_desiredWrite = CDAQ::BLADE_STOP_PWM;

  _motor.attach(motorPin);
  _motor.writeMicroseconds(CDAQ::BLADE_MAX_PWM);
  delay(100);
  _motor.writeMicroseconds(CDAQ::BLADE_STOP_PWM);
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

bool BladeManager::Step(){

  long currentStep = millis();

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

  return true;
}

