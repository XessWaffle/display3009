#ifndef BLADE_FRAME_H
#define BLADE_FRAME_H

#include "ArmFrame.h"


extern void stream(const char* info);

struct ArmFrameNode{
  double theta = 0.0;
  ArmFrame *frame = NULL;

  ArmFrameNode *next = NULL, *prev = NULL;
};


class BladeFrame{

  public:
    BladeFrame();

    void Destroy();

    void AddArmFrame(ArmFrame* frame, double theta);
    void UpdateArmFrame(double theta);
    ArmFrame *GetPrimaryFrame();
    ArmFrame *GetFollowerFrame();
  

  private:
    ArmFrameNode *_root, *_primary = NULL, *_follower = NULL;
    int _frames;
};



#endif


