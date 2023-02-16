#ifndef BLADE_FRAME_H
#define BLADE_FRAME_H

#include "ArmFrame.h"


struct ArmFrameNode{
  double theta = 0.0;
  ArmFrame *frame = NULL;

  ArmFrameNode *next = NULL, *prev = NULL;
};


class BladeFrame{

  public:
    BladeFrame();

    void Destroy();

    void OnFrameEntry();

    ArmFrame *GetArmFrame(double theta);

    void AddArmFrame(ArmFrame* frame, double theta);
    bool UpdatePrimaryFrame(double theta);
    bool UpdateFollowerFrame(double theta);
    ArmFrame *GetPrimaryFrame();
    ArmFrame *GetFollowerFrame();
  

  private:
    ArmFrameNode *_root, *_primary = NULL, *_follower = NULL;
    int _frames;
};



#endif


