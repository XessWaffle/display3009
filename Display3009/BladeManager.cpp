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
  _blade.theta += _blade.omega * 0.00005;
  if(_blade.theta > TWO_PI){
    _blade.theta -= TWO_PI;
    _blade.rotationComplete = true;
  }
  portEXIT_CRITICAL(&critMux);
}


BladeManager::BladeManager(){

}
BladeManager::BladeManager(int motorPin){

  pinMode(LIS331_POWER, OUTPUT);
  pinMode(LIS331_GND, OUTPUT);

  digitalWrite(LIS331_POWER, HIGH);
  digitalWrite(LIS331_GND, LOW);

  this->_motorPin = motorPin;
  this->_motorWriteValue = BLADE_STOP_PWM;

  this->_lastStepped = millis();
  this->_desiredWrite = BLADE_STOP_PWM;

  if(_mpu.begin()){
    _mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    _mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }
  /*_xl.setI2CAddr(0x19);
  _xl.begin(LIS331::USE_I2C);
  _xl.setODR(LIS331::DR_1000HZ);
  _xl.setFullScale(LIS331::LOW_RANGE);*/
  

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isr_integrator, true);
  timerAlarmWrite(timer, 50, true);
  timerAlarmEnable(timer);

  _motor.attach(motorPin);
  _motor.writeMicroseconds(BLADE_MAX_PWM);
  delay(100);
  _motor.writeMicroseconds(BLADE_STOP_PWM);
  
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

double BladeManager::GetAngularVelocity(){
  portENTER_CRITICAL(&critMux);
  double omega = _blade.omega;
  portEXIT_CRITICAL(&critMux);
  return omega;
}

double BladeManager::GetAngularPosition(){
  portENTER_CRITICAL(&critMux);
  double theta = _blade.theta;
  portEXIT_CRITICAL(&critMux);
  return theta;
}

bool BladeManager::IsRotationComplete(){
  portENTER_CRITICAL(&critMux);
  bool complete = _blade.rotationComplete;
  _blade.rotationComplete = false;
  portEXIT_CRITICAL(&critMux);
  return complete;
}

void BladeManager::Step(){

  long currentStep = millis();

  sensors_event_t a, g, temp;
  int16_t x, y, z;
  _mpu.getEvent(&a, &g, &temp);
  //_xl.readAxes(x, y, z);
 //float xly = _xl.convertToG(LIS331::LOW_RANGE, y);

  double omegaLow = sqrt(abs(a.acceleration.y)/MPU_RADIUS);
  double omegaHigh = 0.0; //sqrt(abs(xly)/HPM_RADIUS);
  double omega = omegaLow > BLADE_SATURATION_OMEGA ? omegaHigh : omegaLow;
  portENTER_CRITICAL(&critMux);
  _blade.omega = omega;
  portEXIT_CRITICAL(&critMux);

  Serial.print(a.acceleration.x);
  Serial.print(" ");
  Serial.println(a.acceleration.y);
  Serial.print(" ");
  Serial.println(a.acceleration.z);

  if(this->_state == SpinState::STARTING){
    if(omega > BLADE_START_OMEGA){
      this->_state = SpinState::SPINNING;
    } else if(currentStep - this->_lastStepped > BLADE_UPDATE_DELAY){
      this->_motorWriteValue += 1;

      this->_lastStepped = currentStep;
    }
  } else if(this->_state == SpinState::SPINNING){

    if(currentStep - this->_lastStepped > BLADE_UPDATE_DELAY){
      this->_motorWriteValue += this->_motorWriteValue < this->_desiredWrite ? 1 : -1;

      if(this->_motorWriteValue >= BLADE_MAX_PWM)
        this->_motorWriteValue = BLADE_MAX_PWM;
      
      if(this->_motorWriteValue <= BLADE_STOP_PWM)
        this->_motorWriteValue = BLADE_STOP_PWM;
  

      this->_lastStepped = currentStep;
    }

  } else if(this->_state == SpinState::STOPPING){
    if(omega < BLADE_START_OMEGA){
      this->_state = SpinState::STOPPED;
    } else if(currentStep - this->_lastStepped > BLADE_UPDATE_DELAY){
      this->_motorWriteValue += -1;
      this->_lastStepped = currentStep;
    }
  } else if(this->_state == SpinState::STOPPED){
    this->_motorWriteValue = BLADE_STOP_PWM;
  }
  _motor.writeMicroseconds(this->_motorWriteValue);

}