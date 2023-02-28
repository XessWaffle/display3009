#ifndef ARM_FRAME_H
#define ARM_FRAME_H

#include <FastLED.h>

class ArmFrame : Destructible{

  public:

    ArmFrame();
    ArmFrame(int numLeds);

    void SetLED(int led, struct CRGB color);
    
    void Destroy();
    void Reset();

    void Trigger(struct CRGB *mod);

  private:
    int _numLeds;
    struct CRGB *_leds;

};


#endif