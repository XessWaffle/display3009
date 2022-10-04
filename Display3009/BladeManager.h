#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include <Arduino.h>

#define ISR_HALL_DT 15
#define BLADE_START_PWM 1310
#define BLADE_STOP_PWM 1000
#define BLADE_MAX_PWM 1450
#define BLADE_START_DT 250
#define BLADE_START_DELAY 3000

struct Blade{
  double omega, theta;
  unsigned long lastReadTime;
};

extern void IRAM_ATTR isr_integrator();
extern void IRAM_ATTR isr_hall();

// Interrupts
extern hw_timer_t *timer;
extern portMUX_TYPE critMux;

enum SpinState{STARTING, SPINNING, STOPPING, STOPPED};

class BladeManager{
  public:
    BladeManager();
    BladeManager(int motorPin, int hallPin);

    void StartBlade();
    void StopBlade();

    // Blade API
    void SetTargetVelocity(double omega);
    double GetAngularVelocity();
    double GetAngularPosition();

    void Step();

  private:
    int _motorPin, _hallPin;
    int _motorWriteValue;
    int _state = SpinState::STOPPED;

    long _lastStepped;
    
    double _desiredOmega;

    Servo _motor;

   
};

#endif