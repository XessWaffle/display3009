#include "BladeFrame.h"
#include <Arduino.h>

BladeFrame::BladeFrame(){
  for(int i = 0; i < ARM_FRAME_UNITS; i++){
    this->_set[i] = NULL;
  }
}

void BladeFrame::Destroy(){
  for(int i = 0; i < ARM_FRAME_UNITS; i++){
    ArmFrameNode *head = this->_set[i], *remove = head;

    while(head->next != NULL){
      head = head->next;
      remove->frame->Destroy();
      free(remove);
      remove = head;      
    }
  }
}

void BladeFrame::AddArmFrame(ArmFrame* frame, double theta){
  int index = (int)(theta/TWO_PI * ARM_FRAME_UNITS);
  ArmFrameNode *head = this->_set[index], *edit = (ArmFrameNode*) malloc(sizeof(ArmFrameNode));
  edit->frame = frame;
  edit->theta = theta;
  edit->next = NULL;
  edit->prev = NULL;

  if(head == NULL){
    this->_set[index] = edit;
  } else {

    bool inserted = false;

    while(head->next != NULL){
      if(head->theta > edit->theta){
        ArmFrameNode *prev = head->prev;
        prev->next = edit;
        edit->prev = prev;
        edit->next = head;
        head->prev = edit;
        inserted = true;
      }

      head = head->next;
    }

    if(!inserted){
      head->next = edit;
      edit->prev = head;
    }

  }

}

ArmFrame *BladeFrame::GetArmFrame(double theta, double noise){
  int index = (int)(theta/TWO_PI * ARM_FRAME_UNITS);

  ArmFrameNode *head = this->_set[index];

  while(head != NULL){

    if(theta >= head->theta){

      if(head->next != NULL && theta >= head->next->theta){
        continue;
      } else {
        return head->frame;
      }
    }

    head = head->next;
  }

  return NULL;
}

void BladeFrame::RemoveArmFrame(double theta, double noise){
  int index = (int)(theta/TWO_PI * ARM_FRAME_UNITS);

  ArmFrameNode *head = this->_set[index], *remove = NULL;

  while(head != NULL){

    if(head->theta > theta){
      if(head->prev->theta + noise >= theta){
        remove = head->prev;
        remove->frame->Destroy();
        remove->prev->next = head;
        head->prev = remove->prev;
        free(remove);
      } 
      break;
    }

    head = head->next;
  }


}