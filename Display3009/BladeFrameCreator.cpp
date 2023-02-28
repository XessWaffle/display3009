#include "BladeFrameCreator.h"

#include "Constants.h"

BladeFrameCreator::BladeFrameCreator(FrameDataAllocator *dataAllocator){
  this->_allocator = dataAllocator;
}

bool BladeFrameCreator::StageFrame(int sectors){
  if(this->_sectors != sectors){
    this->_sectors = sectors;

    if(_stagedFrame != NULL)
      this->_allocator->Destroy(_stagedFrame);

    _stagedFrame = this->_allocator->CreateBladeFrame();
  } else {
    _stagedFrame->Clear();
  }

  return true;
}

bool BladeFrameCreator::StageArm(int sector){
  if(this->_stagedFrame == NULL || this->_stagedArm != NULL) return false;
  double theta = (double) sector/this->_sectors * TWO_PI;
  ArmFrame *arm = this->_stagedFrame->GetArmFrame(theta);
  this->_stagedArm = arm != NULL ? arm : this->_allocator->CreateArmFrame();
  if(arm == NULL)
    this->_stagedFrame->AddArmFrame(this->_stagedArm, theta);
  return true;
}

bool BladeFrameCreator::SetLED(int led, struct CRGB color){
  if(this->_stagedArm == NULL) return false;
  this->_stagedArm->SetLED(led, color);
  return true;
}

bool BladeFrameCreator::SetLEDs(int startIndex, int endIndex, struct CRGB color){
  if(this->_stagedArm == NULL) return false;
  for(int i = startIndex; i < endIndex; i++)
    this->_stagedArm->SetLED(i, color);
  return true;
}

bool BladeFrameCreator::CommitArm(){
  this->_stagedArm = NULL;
  return true;
}

BladeFrame *BladeFrameCreator::CommitFrame(){
  return this->_stagedFrame;
}
