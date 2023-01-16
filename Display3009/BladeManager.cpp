#include "BladeManager.h"
#include "LIS331.h"

Blade _blade;
Servo _motor;
Adafruit_MPU6050 _mpu;
LIS331 _xl;
portMUX_TYPE critMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t *timer;

void IRAM_ATTR isr_integrator(){
  portENTER_CRITICAL(&critMux);
  _blade.theta += _blade.omega * 0.00025 * _blade.driftMultiplier;

  if(_blade.theta > TWO_PI){
    _blade.theta -= TWO_PI;
  }

  if(_blade.currFrame != NULL){
    ArmFrame *primaryFrame = NULL, *followerFrame = NULL;
    _blade.currFrame->UpdateArmFrame(_blade.theta);

    primaryFrame = _blade.currFrame->GetPrimaryFrame();
    followerFrame = _blade.currFrame->GetFollowerFrame();

    if(primaryFrame != NULL) {
      primaryFrame->Trigger(_blade.primary);
      _blade.triggered = true;
    }
    if(followerFrame != NULL) {
      followerFrame->Trigger(_blade.follower);
      _blade.triggered = true;
    }
  }
  portEXIT_CRITICAL(&critMux);
}


BladeManager::BladeManager(){

}

BladeManager::BladeManager(int motorPin){

  this->_motorPin = motorPin;
  this->_motorWriteValue = BLADE_STOP_PWM;

  this->_lastStepped = millis();
  this->_lastRead = millis();
  this->_desiredWrite = BLADE_STOP_PWM;

  if(_mpu.begin()){
    _mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    _mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }
  _xl.setI2CAddr(0x19);
  _xl.begin(LIS331::USE_I2C);
  _xl.setODR(LIS331::DR_1000HZ);
  _xl.setFullScale(LIS331::LOW_RANGE);
  
  _motor.attach(motorPin);
  _motor.writeMicroseconds(BLADE_MAX_PWM);
  delay(100);
  _motor.writeMicroseconds(BLADE_STOP_PWM);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isr_integrator, true);
  timerAlarmWrite(timer, 250, true);
  timerAlarmEnable(timer);
  this->_timerDisabled = false;
}

void BladeManager::SetState(SpinState state){
  this->_state = state;
}

int BladeManager::GetState(){
  return this->_state;
}

void BladeManager::StartBlade(){
  if(!this->_timerDisabled)
    this->_state = SpinState::STARTING;
}
void BladeManager::StopBlade(){
  this->_state = SpinState::STOPPING;
}

// Blade API
void BladeManager::SetTrigger(BladeFrame *frame, struct CRGB *primary, struct CRGB *follower){
  portENTER_CRITICAL(&critMux);
  _blade.currFrame = frame;
  _blade.primary = primary;
  _blade.follower = follower;
  portEXIT_CRITICAL(&critMux);
}

void BladeManager::SetTarget(int write){
  this->_desiredWrite = write;
}

void BladeManager::SetDriftMultiplier(double multiplier){
  portENTER_CRITICAL(&critMux);
  _blade.driftMultiplier = multiplier;
  portEXIT_CRITICAL(&critMux);
}

bool BladeManager::IsTriggered(){
  bool triggered = false;
  portENTER_CRITICAL(&critMux);
  triggered = _blade.triggered;
  _blade.triggered = false;
  portEXIT_CRITICAL(&critMux);
  return triggered;
}

double BladeManager::GetAngularVelocity(){
  portENTER_CRITICAL(&critMux);
  double omega = _blade.omega;
  portEXIT_CRITICAL(&critMux);
  return omega;
}

double BladeManager::GetLowAngularVelocity(){
  portENTER_CRITICAL(&critMux);
  double omega = _blade.omegaLow;
  portEXIT_CRITICAL(&critMux);
  return omega;
}

double BladeManager::GetHighAngularVelocity(){
  portENTER_CRITICAL(&critMux);
  double omega = _blade.omegaHigh;
  portEXIT_CRITICAL(&critMux);
  return omega;
}

double BladeManager::GetAngularPosition(){
  portENTER_CRITICAL(&critMux);
  double theta = _blade.theta;
  portEXIT_CRITICAL(&critMux);
  return theta;
}

bool BladeManager::Step(){

  long currentStep = millis();
  bool lowSaturation = this->_desiredWrite > BLADE_SATURATION_PWM;

  sensors_event_t a, g, temp;
  int16_t x, y, z;
  float xlx, xly, xlz;
  double omega;
  bool triggered = false, sensorQuery = currentStep - this->_lastRead > SENSOR_QUERY_TIME;

  if(sensorQuery && this->_velocityUpdateRequest)
    if(!lowSaturation){
      _mpu.getEvent(&a, &g, &temp);
      omega = sqrt(abs(a.acceleration.y)/MPU_RADIUS);
      this->_lastRead = currentStep;
    } else {
      _xl.readAxes(x, y, z);
      xly = _xl.convertToG(LOW_RANGE_G, y);
      omega = sqrt(abs(xly)/HPM_RADIUS);
      this->_lastRead = currentStep;
    }

  portENTER_CRITICAL(&critMux);
  if(_blade.readings == BLADE_READINGS){
    _blade.omega /= BLADE_READINGS;
    this->_velocityUpdateRequest = false;
    _blade.readings = 0;
  }
  if(sensorQuery && this->_velocityUpdateRequest){
    _blade.omega += omega;
    _blade.readings++;
  }
  triggered = _blade.triggered;
  portEXIT_CRITICAL(&critMux);

  if(this->_state == SpinState::STARTING){
    if(this->_motorWriteValue >= BLADE_START_PWM){
      this->_state = SpinState::SPINNING;
      this->_velocityUpdateRequest = true;
    } else if(currentStep - this->_lastStepped > BLADE_UPDATE_DELAY){
      this->_motorWriteValue += 1;
      this->_lastStepped = currentStep;
    }
  } else if(this->_state == SpinState::SPINNING){

    bool desiredValue = this->_desiredWrite == this->_motorWriteValue;

    if(currentStep - this->_lastStepped > BLADE_UPDATE_DELAY && !desiredValue){
      this->_motorWriteValue += this->_motorWriteValue < this->_desiredWrite ? 1 : -1;

      if(this->_motorWriteValue >= BLADE_MAX_PWM)
        this->_motorWriteValue = BLADE_MAX_PWM;
      
      if(this->_motorWriteValue <= BLADE_STOP_PWM)
        this->_motorWriteValue = BLADE_STOP_PWM;
  

      this->_lastStepped = currentStep;
    }

    if(!desiredValue && this->_desiredWrite == this->_motorWriteValue){
      this->_velocityUpdateRequest = true;
    }

  } else if(this->_state == SpinState::STOPPING){
    if(this->_motorWriteValue < BLADE_START_PWM){
      this->_state = SpinState::STOPPED;
    } else if(currentStep - this->_lastStepped > BLADE_UPDATE_DELAY){
      this->_motorWriteValue += -1;
      this->_lastStepped = currentStep;
    }
  } else if(this->_state == SpinState::STOPPED){
    this->_motorWriteValue = BLADE_STOP_PWM;
  }
  _motor.writeMicroseconds(this->_motorWriteValue);
  return triggered;
}

void BladeManager::DisableTimer(){
  if(this->_state == SpinState::STOPPED){
    timerAlarmDisable(timer);
    this->_timerDisabled = true;
  }
}
void BladeManager::EnableTimer(){
  timerAlarmEnable(timer);
  this->_timerDisabled = false;
}