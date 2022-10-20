#ifndef ARM_FRAME_H
#define ARM_FRAME_H

#include <FastLED.h>

class ArmFrame{
  public:

    typedef enum {FLASH, HOLD} frameType;

    ArmFrame();
    ArmFrame(frameType type, int numLeds);

    void SetLED(int led, CRGB color);
    void Destroy();

    void Trigger(struct CRGB *mod);

  private:
    struct CRGB *_ledFrame;
    int _numLeds;
    frameType _type;
};


#endif