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
  _blade.theta += _blade.omega * 0.0015;

  if(_blade.theta > TWO_PI){
    _blade.theta = 0;
  }

  if(_blade.currFrame != NULL){
    double theta = _blade.theta;
    ArmFrame *primaryFrame = _blade.currFrame->GetArmFrame(theta, ANGULAR_FRAME_NOISE);
    theta += PI;
    if(theta >= TWO_PI) theta -= TWO_PI;
    ArmFrame *followerFrame = _blade.currFrame->GetArmFrame(theta, ANGULAR_FRAME_NOISE);

    if(primaryFrame != NULL) primaryFrame->Trigger(_blade.primary);
    if(followerFrame != NULL) followerFrame->Trigger(_blade.follower);
  }
  portEXIT_CRITICAL(&critMux);
}


BladeManager::BladeManager(){

}

BladeManager::BladeManager(int motorPin){

  this->_motorPin = motorPin;
  this->_motorWriteValue = BLADE_STOP_PWM;

  this->_lastStepped = millis();
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
  

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isr_integrator, true);
  timerAlarmWrite(timer, 1500, true);
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
  this->_driftMultiplier = multiplier;
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


double BladeManager::GetCachedData(sensor sensor, sensorData dataType, axis axis){
  if(sensor == DATA_LOW){
    if(dataType == TEMPERATURE){
      return this->_lowTemp.temperature;
    } else if(dataType == GYRO){
      if(axis == X){
        return this->_lowGyro.gyro.x;
      } else if(axis == Y){
        return this->_lowGyro.gyro.y;
      } else if(axis == Z){
        return this->_lowGyro.gyro.z;
      }
    } else if(dataType == ACCELERATION){
      if(axis == X){
        return this->_lowAcceleration.acceleration.x;
      } else if(axis == Y){
        return this->_lowAcceleration.acceleration.y;
      } else if(axis == Z){
        return this->_lowAcceleration.acceleration.z;
      }
    }
  } else {
    if(dataType == ACCELERATION){
      if(axis == X){
        return this->_highX;
      } else if(axis == Y){
        return this->_highY;
      } else if(axis == Z){
        return this->_highZ;
      }
    }
  }

  return 0.0;
}

void BladeManager::Step(){

  long currentStep = millis();

  sensors_event_t a, g, temp;
  _mpu.getEvent(&a, &g, &temp);

  int16_t x, y, z;
  _xl.readAxes(x, y, z);
  float xlx = _xl.convertToG(LOW_RANGE_G, x), xly = _xl.convertToG(LOW_RANGE_G, y), xlz = _xl.convertToG(LOW_RANGE_G, z);

  double omegaLow = sqrt(abs(a.acceleration.y)/MPU_RADIUS);
  double omegaHigh = sqrt(abs(xly)/HPM_RADIUS) * _driftMultiplier;
  double omega = omegaLow > BLADE_SATURATION_OMEGA ? omegaHigh : omegaLow;

  portENTER_CRITICAL(&critMux);
  _blade.omega = omega;
  _blade.omegaLow = omegaLow;
  _blade.omegaHigh = omegaHigh;
  portEXIT_CRITICAL(&critMux);

  this->_lowAcceleration = a;
  this->_lowGyro = g;
  this->_lowTemp = temp;

  this->_highX = xlx;
  this->_highY = xly;
  this->_highZ = xlz;

  //Serial.printf("Low (%.2f): (%.2f, %.2f, %.2f) | High (%.2f) : (%d, %d, %d)\n", omegaLow, a.acceleration.x, a.acceleration.y, a.acceleration.z, omegaHigh, x, y, z);

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