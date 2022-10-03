#include "BladeManager.h"

void IRAM_ATTR isr_integrator(){
  portENTER_CRITICAL(&critMux);
  blade.theta += blade.omega * 0.01;
  portEXIT_CRITICAL(&critMux);
}

void IRAM_ATTR isr_hall(){
  portENTER_CRITICAL(&critMux);
  long dt = millis() - blade.lastReadTime;
  if(dt > ISR_HALL_DT){
    omega = 2 * PI / dt;
    blade.lastReadTime = time;
  }
  portEXIT_CRITICAL(&critMux);
}

BladeManager::BladeManager(){


}

BladeManager::BladeManager(int motorPin, int hallPin){
  this->_motorPin = motorPin;
  this->_hallPin = hallPin;

  this->_desiredOmega = 0;
  this->_motorWriteValue = BLADE_STOP_PWM;

  blade.motor.attach(this->_motorPin);

  blade.writeMicroseconds(2000);
  delay(100);
  blade.writeMicroseconds(1000);
  delay(100);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isr_integrator, true);
  timerAlarmWrite(timer, 10, true);
  timerAlarmEnable(timer);
  
  pinMode(this->_hallPin, INPUT);
  attachInterrupt(this->_hallPin, &isr_hall, FALLING);
}

bool BladeManager::Start(){
  _state = SpinState::STARTING;
}


bool BladeManager::Stop(){
  _state = SpinState::STOPPING;
}

// Blade API
void BladeManager::SetTargetVelocity(double omega){
  this->_desiredOmega = omega;
}

double BladeManager::GetAngularVelocity(){
  return blade.omega * 1000;
}

double BladeManager::GetAngularPosition(){
  return blade.theta;
}


void BladeManager::Step(){
  Servo* motor = &(blade.motor);
  long currentStep = millis();

  if(this->_state == SpinState::STARTING){
    if(currentStep - blade.lastReadTime <= BLADE_START_DT){
      this->_state == Spinstate::SPINNING;
    } else if(currentStep - lastStepped >= BLADE_START_DELAY){
      this->_motorWriteValue = this->_motorWriteValue == BLADE_START_PWM ? BLADE_STOP_PWM : BLADE_START_PWM;
      motor->writeMicroseconds(this->_motorWriteValue);
      lastStepped = millis();
    }
  } else if(this->_state == SpinState::SPINNING){
    double desiredOmegaMS = this->_desiredOmega * 0.001;
    double error = blade.omega - desiredOmegaMS;

    if(error < 0){
      this->_motorWriteValue += this->_motorWriteValue <= BLADE_MAX_PWM;
    } else if(error > 0){
      this->_motorWriteValue -= this->_motorWriteValue >= BLADE_STOP_PWM;
    }

    motor->writeMicroseconds(this->_motorWriteValue);

  } else if(this->_state == SpinState::STOPPING){
    if(currentStep - blade.lastReadTime >= BLADE_START_DT){
      this->_motorWriteValue = BLADE_STOP_PWM;
      this->_state = SpinState::STOPPED;
    } else {
      this->_motorWriteValue -= this->_motorWriteValue >= BLADE_STOP_PWM;
    }

    motor->writeMicroseconds(this->_motorWriteValue);
  }

}