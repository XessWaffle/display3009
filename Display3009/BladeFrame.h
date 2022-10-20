#ifndef BLADE_FRAME_H
#define BLADE_FRAME_H

#define ARM_FRAME_UNITS 360

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

    void AddArmFrame(ArmFrame* frame, double theta);
    ArmFrame *GetArmFrame(double theta, double noise);
    void RemoveArmFrame(double theta, double noise);

  private:
    ArmFrameNode *_set[ARM_FRAME_UNITS];

};



#endif


