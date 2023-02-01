#include "BladeManager.h"
#include "LIS331.h"

Servo _motor;
const int MPU = 0x68;
LIS331 _xl;

BladeManager::BladeManager(){

}

BladeManager::BladeManager(int motorPin){

  this->_motorPin = motorPin;
  this->_motorWriteValue = CDAQ::BLADE_STOP_PWM;

  this->_lastStepped = millis();
  this->_lastTrigger = millis();
  this->_desiredWrite = CDAQ::BLADE_STOP_PWM;

  Wire.begin();
  Wire.setClock(200000);
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

   // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x18);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);

  _xl.setI2CAddr(0x19);
  _xl.begin(LIS331::USE_I2C);
  _xl.setODR(LIS331::DR_1000HZ);
  _xl.setFullScale(LIS331::LOW_RANGE);
  
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

  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 2, true); // Read 2 registers total
  bool accX = ((Wire.read() << 8 | Wire.read()) & 1 << 16) != 0; // X-axis value

  if(!_prevAcceleration && accX && !this->_triggerLatch) {
    this->_zeroTrigger += 1;
    this->_zeroTrigger %= 2;
    this->_triggerLatch = true;
    this->_lastTrigger = currentStep;
  } else if(!_prevAcceleration && accX && currentStep - this->_lastTrigger > CDAQ::SENSOR_TRIGGER_DELAY) {
    this->_triggerLatch = false;
  }

  _prevAcceleration = accX;

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

  return !this->_zeroTrigger;
}

