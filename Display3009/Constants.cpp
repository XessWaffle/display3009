#include "Constants.h"

/*CDAQ::LOW_RANGE_G           = 100;
CDAQ::MID_RANGE_G           = 200;
CDAQ::HIGH_RANGE_G          = 400;
CDAQ::BLADE_STOP_PWM        = 1000;
CDAQ::BLADE_START_PWM       = 1250;
CDAQ::BLADE_MAX_PWM         = 1450;
CDAQ::MPU_RADIUS            = 0.084;
CDAQ::HPM_RADIUS            = 0.109;
CDAQ::BLADE_UPDATE_DELAY    = 50;
CDAQ::GRAVITY               = 9.80665;
CDAQ::SENSOR_TRIGGER_DELAY  = 15;


COPS::DATA_PIN_FOLLOWER     = 33;
COPS::CLOCK_PIN_FOLLOWER    = 32;
COPS::DATA_PIN_PRIMARY      = 19;
COPS::CLOCK_PIN_PRIMARY     = 14;
COPS::MOTOR_PIN             = 13;

  
CRENDER::NUM_LEDS           = 72;
CRENDER::NUM_ANIMATIONS     = 6;

CCOMMS::SSID                = "3009 Xess";
CCOMMS::PWD                 = "dingd0ngditch";
CCOMMS::PORT                = 80;
CCOMMS::XESSAMD             = "192.168.137.71";
CCOMMS::MESSAGE_PAUSE       = 0xFF;
CCOMMS::REFRESH             = 0xFF;
CCOMMS::DISCONNECT          = 0xFE;
CCOMMS::THROTTLE            = createWriteInstruction(0x00);
CCOMMS::START               = createWriteInstruction(0x01);
CCOMMS::STOP                = createWriteInstruction(0x02);
CCOMMS::TEST                = createReadInstruction(0x03);
CCOMMS::STATE               = createReadInstruction(0x05);
CCOMMS::ANIMATION           = createWriteInstruction(0x06);
CCOMMS::FPS                 = createWriteInstruction(0x07);
CCOMMS::MULTIPLIER          = createWriteInstruction(0x08);
CCOMMS::NUM_INST            = 8;
CCOMMS::ID                  = 1;*/



uint8_t CCOMMS::createReadInstruction(uint8_t inst){ return (uint8_t) (inst | 0x1 << 7); }
uint8_t CCOMMS::createWriteInstruction(uint8_t inst){ return (uint8_t) (inst & ~(0x1 << 7)); }
