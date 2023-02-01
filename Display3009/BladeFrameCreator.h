#ifndef BLADE_FRAME_CREATOR_H
#define BLADE_FRAME_CREATOR_H

#include "BladeFrame.h"
#include "Constants.h"

class BladeFrameCreator{
  public:
    BladeFrameCreator();

    bool StageFrame();
    bool StageArm(double theta);
    bool SetLED(int index, struct CRGB color);
    bool CommitArm();
    BladeFrame *CommitFrame();

  private:
    BladeFrame *_stagedFrame = NULL;
    ArmFrame *_stagedArm = NULL;
};


#endif