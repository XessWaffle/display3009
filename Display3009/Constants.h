#pragma once

#define CDAQ Constants::DAQ
#define COPS Constants::OPS
#define CRENDER Constants::RENDER
#define CCOMMS Constants::COMMS
#define CVAL Constants::VALUE
#define CALLOC Constants::ALLOCATION


#include <stdint.h>

namespace Constants{

  namespace DAQ{
    static const int LOW_RANGE_G          = 100;
    static const int MID_RANGE_G          = 200;
    static const int HIGH_RANGE_G         = 400;

    static const int BLADE_STOP_PWM       = 1000;
    static const int BLADE_START_PWM      = 1250;
    static const int BLADE_MAX_PWM        = 1450;

    static const int BLADE_UPDATE_DELAY   = 50;

    static const int HALL_ENTRY_THRESHOLD = -20;
    static const int HALL_EXIT_THRESHOLD  = 0;
    static const float HALL_SWEEP         = 0.3246312;
    static const float HALL_ANGLE         = 1.3090006122;

    static const int FILTER_NODES         = 6;
  };

  namespace OPS{
    static const int DATA_PIN_FOLLOWER  = 23; // VSPI
    static const int CLOCK_PIN_FOLLOWER = 18;
    static const int DATA_PIN_PRIMARY   = 13; // HSPI
    static const int CLOCK_PIN_PRIMARY  = 14;
    static const int MOTOR_PIN          = 25;
  };

  namespace RENDER{
    static const int NUM_LEDS          = 72;
    static const int NUM_ANIMATIONS    = 7;

    static const int BUFFER_SIZE       = 297;
  };

  namespace COMMS{
    uint8_t createReadInstruction(uint8_t inst);
    uint8_t createWriteInstruction(uint8_t inst);

    static const char* SSID            = "3009 Xess";
    static const char* PWD             = "dingd0ngditch";
    static const int PORT              = 80;
    static const char* XESSAMD         = "192.168.137.202";
    static const uint8_t MESSAGE_PAUSE = 0xFF;
    static const uint8_t REFRESH       = 0xFF;
    static const uint8_t DISCONNECT    = 0xFE;
    static const uint8_t THROTTLE      = createWriteInstruction(0x00);
    static const uint8_t START         = createWriteInstruction(0x01);
    static const uint8_t STOP          = createWriteInstruction(0x02);
    static const uint8_t TEST          = createReadInstruction(0x03);
    static const uint8_t STATE         = createReadInstruction(0x05);
    static const uint8_t ANIMATION     = createWriteInstruction(0x06);
    static const uint8_t FPS           = createWriteInstruction(0x07);
    static const uint8_t MULTIPLIER    = createWriteInstruction(0x08);
    static const uint8_t STAGE_FRAME   = createWriteInstruction(0x09);
    static const uint8_t STAGE_ARM     = createWriteInstruction(0x0A);
    static const uint8_t SET_LED       = createWriteInstruction(0x0B);
    static const uint8_t SET_LEDS      = createWriteInstruction(0x0E);
    static const uint8_t COMMIT_ARM    = createWriteInstruction(0x0C);
    static const uint8_t COMMIT_FRAME  = createWriteInstruction(0x0D);
    static const uint8_t NUM_INST      = 14;
    static const uint8_t ID            = 1;

  };

  namespace VALUE{
    static const int MHZ      = 1000000;
    static const float MICROS = 0.000001;
  }

  namespace ALLOCATION{
    static const int ALLOCATABLE_DESTRUCTIBLES = 2000;
  }
};
