#include "BladeFrame.h"

BladeFrame::BladeFrame(){
  this->_root = NULL;
  this->_primary = NULL;
  this->_follower = NULL;
  this->_frames = 0;
}

void BladeFrame::Destroy(){
  ArmFrameNode *head = this->_root->next;

  while(head->next != this->_root){
    ArmFrameNode* remove = head;
    head = head->next;
    remove->frame->Destroy();
    free(remove);
  }

  this->_root->frame->Destroy();
  free(this->_root);

}


void BladeFrame::AddArmFrame(ArmFrame* frame, double theta){
  
  while(theta < 0) theta += TWO_PI;
  while(theta >= TWO_PI) theta -= TWO_PI;

  struct ArmFrameNode* next = (struct ArmFrameNode *) malloc(sizeof(struct ArmFrameNode)), *head = this->_root;

  next->theta = theta;
  next->frame = frame;

  if(head == NULL){
    this->_root = next;
    next->next = next;
    next->prev = next;
  } else {
    int count = 0;
    while(true){
      
      if(head->theta > theta || count == this->_frames){
        struct ArmFrameNode *prev = head->prev;
        if(prev != NULL)  prev->next = next;
        next->prev = prev;
        next->next = head;
        head->prev = next;
        break;
      }

      head = head->next;
      count++;

    }
  }

  this->_frames++;

}

uint8_t BladeFrame::UpdateArmFrame(double theta){

  double followerTheta = theta + PI;

  while(theta < 0) theta += TWO_PI;
  while(theta >= TWO_PI) theta -= TWO_PI;
  while(followerTheta >= TWO_PI) followerTheta -= TWO_PI;
  
  ArmFrameNode *tempPrimary = this->_primary, *tempFollower = this->_follower;

  if(this->_primary == NULL) this->_primary = this->_root;
  if(this->_follower == NULL) this->_follower = this->_root;

  int count = 0;

  while(count < this->_frames) {
    if(this->_primary->theta <= theta && 
      (this->_primary->next->theta < this->_primary->theta || this->_primary->next->theta > theta))
        break;
    this->_primary = this->_primary->next;
    count++;
  }
  count = 0;
  while(count < this->_frames) {
    if(this->_follower->theta <= followerTheta && 
      (this->_follower->next->theta < this->_follower->theta || this->_follower->next->theta > followerTheta))
        break;
    this->_follower = this->_follower->next; 
    count++;
  }

  return (uint8_t) ((this->_primary != tempPrimary) << 1) | (this->_follower != tempFollower);

}
 
ArmFrame *BladeFrame::GetPrimaryFrame(){
  return this->_primary->frame;
}

ArmFrame *BladeFrame::GetFollowerFrame(){
  return this->_follower->frame;
}