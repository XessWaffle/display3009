#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "BladeFrame.h"


#define MULTIPLIER_LOW 1.0
#define MULTIPLIER_DIVISIONS 0.001

#define LOW_RANGE_G 100
#define MID_RANGE_G 200
#define HIGH_RANGE_G 400

#define BLADE_STOP_PWM 1000
#define BLADE_MAX_PWM 1450
#define MPU_RADIUS 0.084
#define HPM_RADIUS 0.109

#define BLADE_UPDATE_DELAY 50

#define BLADE_START_OMEGA 10
#define BLADE_SATURATION_OMEGA 40

struct Blade{
  double omega, theta;
  double omegaLow, omegaHigh;
  unsigned long lastReadTime;
  bool rotationComplete = false;
};

extern void IRAM_ATTR isr_integrator();

// Interrupts
extern hw_timer_t *timer;
extern portMUX_TYPE critMux;

enum SpinState{STARTING, SPINNING, STOPPING, STOPPED};

class BladeManager{
  public:
    
    typedef enum {DATA_LOW, DATA_HIGH} sensor;
    typedef enum {ACCELERATION, TEMPERATURE, GYRO} sensorData;
    typedef enum {X, Y, Z} axis;
    
    BladeManager();
    BladeManager(int motorPin);

    void StartBlade();
    void StopBlade();
    void SetState(SpinState state);
    int GetState();

    // Blade API
    void SetTarget(int write);
    void SetDriftMultiplier(double multiplier);
    double GetAngularVelocity();
    double GetAngularPosition();
    double GetLowAngularVelocity();
    double GetHighAngularVelocity();
    double GetCachedData(sensor sensor, sensorData dataType, axis axis);

    bool IsRotationComplete();

    double Step();

  private:
    int _motorPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;
    int _numLeds;

    long _lastStepped;
    
    double _desiredWrite, _driftMultiplier = 1.0;

    sensors_event_t _lowAcceleration, _lowGyro, _lowTemp;
    float _highX, _highY, _highZ;
};

#endif