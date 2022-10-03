#ifndef BLADE_MANAGER_H
#define BLADE_MANAGER_H

#include <ESP32Servo.h>
#include <Arduino.h>

#define ISR_HALL_DT 15
#define BLADE_START_PWM 1310
#define BLADE_STOP_PWM 1000
#define BLADE_START_DT 250
#define BLADE_START_DELAY 3000

struct Blade{
  Servo motor;
  double omega, theta;
  unsigned long lastReadTime;
}

void IRAM_ATTR isr_integrator();
void IRAM_ATTR isr_hall();

volatile struct Blade blade;    

enum SpinState{STARTING, STOPPING, SPINNING, STOPPED};

// Interrupts
hw_timer_t *timer = NULL;
portMUX_TYPE critMux = portMUX_INITIALIZER_UNLOCKED;

class BladeManager{
  public:
    BladeManager();
    BladeManager(int motorPin, int hallPin);

    bool Start();
    bool Stop();

    // Blade API
    void SetTargetVelocity(double omega);
    double GetAngularVelocity();
    double GetAngularPosition();

    void Step();

  private:
    int _motorPin, _hallPin;
    int _motorWriteValue;

    long _lastStepped;
    
    double _desiredOmega;

    SpinState _state;

   
};

#endif