#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define LIS331_POWER 16
#define LIS331_GND 4

#define BLADE_STOP_PWM 1000
#define BLADE_MAX_PWM 1450
#define MPU_RADIUS 0.085
#define HPM_RADIUS 0.124

#define BLADE_UPDATE_DELAY 50

#define BLADE_START_OMEGA 10
#define BLADE_SATURATION_OMEGA 40

struct Blade{
  double omega, theta;
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
    BladeManager();
    BladeManager(int motorPin);

    void StartBlade();
    void StopBlade();
    void SetState(SpinState state);
    int GetState();

    // Blade API
    void SetTarget(int write);
    double GetAngularVelocity();
    double GetAngularPosition();

    bool IsRotationComplete();

    void Step();

  private:
    int _motorPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;

    long _lastStepped;
    
    double _desiredWrite;

    bool _mpuStarted = false;
};

#endif