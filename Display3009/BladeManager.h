#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include <Arduino.h>
#include <Wire.h>
#include "BladeFrame.h"

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

    bool Step();

  private:

    bool _triggerLatch = false;
    bool _prevAcceleration = false;
    int _zeroTrigger = 0;

    int _motorPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;
    int _numLeds;

    long _lastStepped, _lastTrigger;
    
    double _desiredWrite;
};

#endif