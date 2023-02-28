#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include "BladeFrame.h"

enum SpinState{STARTING, SPINNING, STOPPING, STOPPED};

struct BladeData{
  float omega;
  bool trigger;
};

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

    BladeData *Step();

  private:
    struct RingNode{
      int data = 0;
      RingNode *next, *prev;
    };

  private:

    BladeData *_data;
    RingNode *_hallFilter;
    int _hallFilterSum = 0;

    int _motorPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;
    int _numLeds;

    uint8_t _triggerState = 0;
    long _triggerStart = 0;

    long _lastStepped;
    
    double _desiredWrite;
};

#endif