#include "BladeFrameCreator.h"

BladeFrameCreator::BladeFrameCreator();

bool BladeFrameCreator::StageFrame(){
  if(this->_stagedFrame != NULL) return false;
  _stagedFrame = new BladeFrame();
  return true;
}

bool BladeFrameCreator::StageArm(double theta){
  if(this->_stagedFrame != NULL || this->_stagedArm != NULL) return false;
  this->_stagedArm = new ArmFrame(CRENDER::NUM_LEDS);
  this->_stagedFrame->AddArmFrame(this->_stagedArm, theta);
  return true;
}

bool SetLED(int index, struct CRGB color){
  if(this->_stagedArm == NULL) return false;
  this->_stagedArm->SetLED(index, color);
}

bool BladeFrameCreator::CommitArm(){
  this->_stagedArm = NULL;
}

BladeFrame *BladeFrameCreator::CommitFrame(){
  BladeFrame *_staged = this->_stagedFrame;
  this->_stagedFrame = NULL;
  return *_staged;
}
