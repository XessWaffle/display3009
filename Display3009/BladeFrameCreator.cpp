#include "BladeFrameCreator.h"

#include "Constants.h"

BladeFrameCreator::BladeFrameCreator(){

}

bool BladeFrameCreator::StageFrame(int sectors){
  if(this->_stagedFrame != NULL) return false;
  this->_sectors = sectors;
  _stagedFrame = new BladeFrame();
  return true;
}

bool BladeFrameCreator::StageArm(int sector){
  if(this->_stagedFrame == NULL || this->_stagedArm != NULL) return false;
  double theta = (double) sector/this->_sectors * TWO_PI;
  this->_stagedArm = new ArmFrame(CRENDER::NUM_LEDS);
  this->_stagedFrame->AddArmFrame(this->_stagedArm, theta);
  return true;
}

bool BladeFrameCreator::SetLED(int index, struct CRGB color){
  if(this->_stagedArm == NULL) return false;
  this->_stagedArm->SetLED(index, color);
  return true;
}

bool BladeFrameCreator::CopyArm(int sector){
  if(this->_stagedFrame == NULL || this->_stagedArm == NULL) return false;
  double theta = (double) sector/this->_sectors * TWO_PI;
  ArmFrame *toCopy = this->_stagedFrame->GetArmFrame(theta);
  if(toCopy != NULL){
    for(int i = 0; i < CRENDER::NUM_LEDS; i++)
      this->_stagedArm->SetLED(i, toCopy->GetLED(i));
    return true;
  }
  return false;
}

bool BladeFrameCreator::CommitArm(){
  this->_stagedArm = NULL;
  return true;
}

BladeFrame *BladeFrameCreator::CommitFrame(){
  BladeFrame *_staged = this->_stagedFrame;
  this->_stagedFrame = NULL;
  return _staged;
}
