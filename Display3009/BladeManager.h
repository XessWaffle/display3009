#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "BladeFrame.h"


#define LOW_RANGE_G 100
#define MID_RANGE_G 200
#define HIGH_RANGE_G 400

#define BLADE_STOP_PWM 1000
#define BLADE_START_PWM 1250
#define BLADE_MAX_PWM 1450
#define MPU_RADIUS 0.084
#define HPM_RADIUS 0.109

#define BLADE_UPDATE_DELAY 50

#define GRAVITY 9.80665

#define SENSOR_QUERY_DELAY 1000

#define OMEGA_RING_NODES 10

enum SpinState{STARTING, SPINNING, STOPPING, STOPPED};

class BladeManager{
  public:
    BladeManager();
    BladeManager(int motorPin);

    void StartBlade();
    void StopBlade();
    void SetState(SpinState state);
    int GetState();

    // Blade API
    void SetTrigger(BladeFrame *frame, struct CRGB *primary, struct CRGB *follower);
    void SetTarget(int write);

    double GetAngularVelocity();

    bool Step();

  private:
    struct RingNode{
      float data = 0;
      RingNode *prev, *next;
    };

  private:

    bool _timerDisabled;

    int _motorPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;
    int _numLeds;

    long _lastStepped, _lastRead, _lastUpdated;
    long _omegaSum = 0;
    
    double _desiredWrite;

    RingNode *_omega;
};

#endif