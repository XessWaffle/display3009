#include "BladeManager.h"

extern hw_timer_t *timer = NULL;
extern portMUX_TYPE critMux = portMUX_INITIALIZER_UNLOCKED;
struct Blade blade;

void IRAM_ATTR isr_integrator(){
  portENTER_CRITICAL(&critMux);
  blade.theta += blade.omega * 0.01;
  portEXIT_CRITICAL(&critMux);
}

BladeManager::BladeManager(){


}

BladeManager::BladeManager(int motorPin, int hallPin){
  this->_motorPin = motorPin;
  this->_hallPin = hallPin;

  this->_desiredOmega = 0;
  this->_motorWriteValue = BLADE_STOP_PWM;

  this->_motor.attach(motorPin);

  this->_motor.writeMicroseconds(2000);
  delay(100);
  this->_motor.writeMicroseconds(1000);
  delay(100);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isr_integrator, true);
  timerAlarmWrite(timer, 10, true);
  timerAlarmEnable(timer);
}

void BladeManager::StartBlade(){
  this->_state = SpinState::STARTING;
}


void BladeManager::StopBlade(){
  this->_state = SpinState::STOPPING;
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
  long currentStep = millis();

  long hallState = analogRead(this->_hallPin) == 0;

  char buff[255];

  sprintf(buff, "%d, %d, %f, %f\n", this->_state, this->_motorWriteValue, blade.omega, blade.theta);
  Serial.print(buff);  

  if(hallState && !this->_prevHallState){
    long dt = currentStep - blade.lastReadTime;
    if(dt > HALL_DEBOUNCE_DT){
      blade.omega = 2 * PI / dt;
      blade.lastReadTime = currentStep;
      blade.theta = 0;
    }
  }

  this->_prevHallState = hallState;

  if(this->_state == SpinState::STARTING){
    if(blade.omega >= BLADE_START_OMEGA){
      this->_state = SpinState::SPINNING;
    } else if(currentStep - this->_lastStepped >= BLADE_START_DELAY){
      this->_motorWriteValue = this->_motorWriteValue != BLADE_START_PWM ? BLADE_START_PWM : BLADE_STOP_PWM;
      this->_lastStepped = millis();
    }
  } else if(this->_state == SpinState::SPINNING){
    double desiredOmegaMS = this->_desiredOmega * 0.001;
    double error = blade.omega - desiredOmegaMS;

    if(error < 0){
      this->_motorWriteValue += this->_motorWriteValue <= BLADE_MAX_PWM;
    } else if(error > 0){
      this->_motorWriteValue -= this->_motorWriteValue >= BLADE_STOP_PWM;
    }

  } else if(this->_state == SpinState::STOPPING){
    if(blade.omega < BLADE_START_OMEGA){
      this->_motorWriteValue = BLADE_STOP_PWM;
      this->_state = SpinState::STOPPED;
    } else {
      this->_motorWriteValue -= this->_motorWriteValue >= BLADE_STOP_PWM;
    }
  } else if(this->_state == SpinState::STOPPED){
    blade.omega = 0;
    blade.theta = 0;
  }


  this->_motor.writeMicroseconds(this->_motorWriteValue);

}