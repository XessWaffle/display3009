#ifndef ARM_FRAME_H
#define ARM_FRAME_H

#include <FastLED.h>
#include "Constants.h"
#include "Destructible.h"

class ArmFrame : public Destructible{

  public:

    ArmFrame();

    void SetLED(int led, struct CRGB color);
    
    void Destroy();
    void Reset();

    void Trigger(struct CRGB *mod);

  private:
    struct CRGB _leds[CRENDER::NUM_LEDS];

};


#endif