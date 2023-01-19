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

#define SENSOR_QUERY_TIME 50

#define BLADE_STOP_PWM 1000
#define BLADE_START_PWM 1250
#define BLADE_MAX_PWM 1450
#define MPU_RADIUS 0.084
#define HPM_RADIUS 0.109

#define BLADE_UPDATE_DELAY 50

#define ROTATION_RATE 30

#define AVG_READINGS 20


extern void stream(const char* info);

struct Blade{
  double omega, theta;

  BladeFrame *currFrame = NULL;
  struct CRGB *primary = NULL, *follower = NULL;
  bool triggered = false;

  int readings = 0;
};

extern void IRAM_ATTR isr_integrator();

// Interrupts
extern hw_timer_t *timer;
extern portMUX_TYPE critMux;

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
    void SetDriftMultiplier(double multiplier);

    bool IsTriggered();

    double GetAngularVelocity();
    double GetAngularPosition();

    bool Step();

    void DisableTimer();
    void EnableTimer();

  private:
    void UpdateGravity();

  private:

    bool _timerDisabled, _velocityUpdateRequest;

    int _motorPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;
    int _numLeds;

    long _lastStepped, _lastRead;
    
    double _desiredWrite;

    double _gravity = 9.80665;
};

#endif