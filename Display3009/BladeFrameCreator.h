#ifndef BLADE_FRAME_CREATOR_H
#define BLADE_FRAME_CREATOR_H

#include "FrameDataAllocator.h"

class BladeFrameCreator{
  public:
    BladeFrameCreator();
    BladeFrameCreator(FrameDataAllocator *dataAllocator);

    bool StageFrame(int sectors);
    bool StageArm(int sector);
    bool SetLED(int led, struct CRGB color);
    bool SetLEDs(int startIndex, int endIndex, struct CRGB color);
    bool CommitArm();
    BladeFrame *CommitFrame();

  private:
    FrameDataAllocator *_allocator;

    BladeFrame *_stagedFrame = NULL;
    ArmFrame *_stagedArm = NULL;

    int _sectors;
};


#endif