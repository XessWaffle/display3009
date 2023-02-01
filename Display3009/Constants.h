#ifndef CONSTANTS_H
#define CONSTANTS_H

#define CDAQ Constants::DAQ
#define COPS Constants::OPS
#define CRENDER Constants::RENDER
#define CCOMMS Constants::COMMS

namespace Constants{

  namespace DAQ{
    const int LOW_RANGE_G = 100;
    const int MID_RANGE_G = 200;
    const int HIGH_RANGE_G = 400;

    const int BLADE_STOP_PWM = 1000;
    const int BLADE_START_PWM = 1250;
    const int BLADE_MAX_PWM = 1450;
    const double MPU_RADIUS = 0.084;
    const double HPM_RADIUS = 0.109;

    const int BLADE_UPDATE_DELAY = 50;

    const double GRAVITY = 9.80665;

    const int SENSOR_TRIGGER_DELAY = 15;
  };

  namespace OPS{
    const int DATA_PIN_FOLLOWER = 33;
    const int CLOCK_PIN_FOLLOWER = 32;
    const int DATA_PIN_PRIMARY = 19;
    const int CLOCK_PIN_PRIMARY = 14;
    const int MOTOR_PIN = 13;
  };

  namespace RENDER{
    const int NUM_LEDS = 72;
    const int NUM_ANIMATIONS = 6;
  }

  namespace COMMS{
    uint8_t createReadInstruction(uint8_t inst){ return (uint8_t) (inst | 0x1 << 7); }
    uint8_t createWriteInstruction(uint8_t inst){ return (uint8_t) (inst & ~(0x1 << 7)); }
    uint8_t createStreamClient(uint8_t id) { return (uint8_t) (id | 0x1 << 7); }

    const char* SSID            = "3009 Xess";
    const char* PWD             = "dingd0ngditch";
    const int PORT              = 80;
    const char* XESSAMD         = "192.168.137.71";
    const uint8_t MESSAGE_PAUSE = 0xFF;
    const uint8_t REFRESH       = 0xFF;
    const uint8_t DISCONNECT    = 0xFE;
    const uint8_t THROTTLE      = createWriteInstruction(0x00);
    const uint8_t START         = createWriteInstruction(0x01);
    const uint8_t STOP          = createWriteInstruction(0x02);
    const uint8_t TEST          = createReadInstruction(0x03);
    const uint8_t STATE         = createReadInstruction(0x05);
    const uint8_t ANIMATION     = createWriteInstruction(0x06);
    const uint8_t FPS           = createWriteInstruction(0x07);
    const uint8_t MULTIPLIER    = createWriteInstruction(0x08);
    const uint8_t NUM_INST      = 8;

    const uint8_t ID            = 1;

  }
};

#endif