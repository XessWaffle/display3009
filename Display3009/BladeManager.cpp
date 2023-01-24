#include "BladeManager.h"
#include "LIS331.h"

Servo _motor;
Adafruit_MPU6050 _mpu;
LIS331 _xl;

BladeManager::BladeManager(){

}

BladeManager::BladeManager(int motorPin){

  this->_motorPin = motorPin;
  this->_motorWriteValue = BLADE_STOP_PWM;

  this->_lastStepped = millis();
  this->_lastRead = millis();
  this->_lastUpdated = micros();
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

  this->_omega = (RingNode*) malloc(sizeof(RingNode));
  this->_omega->next = this->_omega;
  this->_omega->prev = this->_omega;
  this->_omega->data = 0;
  
  RingNode *head = this->_omega;

  for(int i = 1; i < OMEGA_RING_NODES; i++) {
    RingNode *next = (RingNode*) malloc(sizeof(RingNode));
    next->data = 0;
    next->next = this->_omega;
    head->next = next;
    next->prev = head;
    head = next;
  }
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
void BladeManager::SetTarget(int write){
  this->_desiredWrite = write;
}


double BladeManager::GetAngularVelocity(){
  double omega = this->_omegaSum / OMEGA_RING_NODES;
  return omega;
}

bool BladeManager::Step(){

  long currentStep = millis();

  int16_t x, y, z;
  float xlx, xly, xlz;
  double omega = -1.0;
  bool sensorQuery = currentStep - this->_lastRead > SENSOR_QUERY_DELAY;

  if(sensorQuery) {
    _xl.readAxes(x, y, z);
    xly = _xl.convertToG(LOW_RANGE_G, y) * GRAVITY;
    omega = sqrt(abs(xly) / HPM_RADIUS);
    this->_omegaSum -= this->_omega->data;
    this->_omegaSum += omega;
    this->_omega->data = omega;
    this->_omega = this->_omega->next;
    this->_lastRead = currentStep;
  }

  if(this->_state == SpinState::STARTING){

    if(this->_motorWriteValue >= BLADE_START_PWM){
      this->_state = SpinState::SPINNING;
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

  return omega > 0.0;
}

