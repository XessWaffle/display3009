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

void BladeFrame::UpdateArmFrame(double theta){

  double followerTheta = theta + PI;

  while(theta < 0) theta += TWO_PI;
  while(theta >= TWO_PI) theta -= TWO_PI;
  while(followerTheta >= TWO_PI) followerTheta -= TWO_PI;

  if(this->_primary == NULL) this->_primary = this->_root;
  if(this->_follower == NULL) this->_follower = this->_root;

  int count = 0;

  while(this->_primary->theta > theta && count < this->_frames) {
    this->_primary = this->_primary->next;
    count++;
  }
  count = 0;
  while(this->_follower->theta > followerTheta && count < this->_frames) {
    this->_follower = this->_follower->next; 
    count++;
  }

}
 
ArmFrame *BladeFrame::GetPrimaryFrame(){
  return this->_primary->frame;
}

ArmFrame *BladeFrame::GetFollowerFrame(){
  return this->_follower->frame;
}