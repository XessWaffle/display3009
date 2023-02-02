#ifndef BLADE_FRAME_CREATOR_H
#define BLADE_FRAME_CREATOR_H

#include "BladeFrame.h"

class BladeFrameCreator{
  public:
    BladeFrameCreator();

    bool StageFrame(int sectors);
    bool StageArm(int sector);
    bool SetLED(int index, struct CRGB color);
    bool CommitArm();
    BladeFrame *CommitFrame();

  private:
    BladeFrame *_stagedFrame = NULL;
    ArmFrame *_stagedArm = NULL;

    int _sectors;
};


#endif