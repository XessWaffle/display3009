#include "BladeFrameIterator.h"

BladeFrameIterator::BladeFrameIterator(){
  this->_lastFrameUpdate = millis();
  this->_animation = NULL;
}

BladeFrameIterator::BladeFrameIterator(FILE *animation){
  this->_lastFrameUpdate = millis();
  this->_animation = animation;
} 

void BladeFrameIterator::Destroy(){
  BladeFrameNode *head = this->_frameSet, *remove = head;

  int removed = 0;

  while(removed < this->_frames){
    remove->frame->Mark();
    head = head->next;
    free(remove);
    remove = head;
    removed++;
  }
}

void BladeFrameIterator::SetFile(FILE *animation){

  //TODO Reset frames and loader

  this->_animation = animation;
}

BladeFrame *BladeFrameIterator::GetFrame(){
  if(this->_frames > 0)
    return this->_frameDisplay->frame;
  return NULL;
}


bool BladeFrameIterator::Step(){
  long currentTime = millis();

  bool frameChanged = false;

  if(currentTime - this->_lastFrameUpdate > this->_frameWait && this->_frames > 0){

    int prevId = this->_frameDisplay->id;

    if(this->_frameCounter >= this->_frames - 1){
      this->_forward = !this->_forward;
      this->_frameCounter = 0;
    }
  
    if(this->_type == animationType::LOOP){
      this->_frameDisplay = this->_frameDisplay->next;
    } else if(this->_type == animationType::REWIND){
      this->_frameDisplay = this->_forward ? this->_frameDisplay->next : this->_frameDisplay->prev;
    } else if(this->_type == animationType::STREAM){
      if(this->_frameDisplay->next != NULL){
        BladeFrameNode *remove = this->_frameDisplay;

        this->_frameDisplay = this->_frameDisplay->next;
        this->_frameSet = this->_frameDisplay;

        this->_frameSet->prev = NULL;
        free(remove);
        this->_frames--;
      }
    }

    int currId = this->_frameDisplay->id;

    frameChanged = prevId != currId;
    this->_frameCounter++;
    this->_lastFrameUpdate = currentTime;

    if(frameChanged)
      this->_frameDisplay->frame->OnFrameEntry();
  }
  return frameChanged;
} 

bool BladeFrameIterator::ForceStep(){

  bool frameChanged = false;  

  int prevId = this->_frameDisplay->id;

  if(this->_frameCounter >= this->_frames - 1){
    this->_forward = !this->_forward;
    this->_frameCounter = 0;
  }

  if(this->_type == animationType::LOOP){
    this->_frameDisplay = this->_frameDisplay->next;
  } else if(this->_type == animationType::REWIND){
    this->_frameDisplay = this->_forward ? this->_frameDisplay->next : this->_frameDisplay->prev;
  } else if(this->_type == animationType::STREAM){
    if(this->_frameDisplay->next != NULL){
      BladeFrameNode *remove = this->_frameDisplay;

      this->_frameDisplay = this->_frameDisplay->next;
      this->_frameSet = this->_frameDisplay;

      this->_frameSet->prev = NULL;
      free(remove);
      this->_frames--;
    }
  }

  int currId = this->_frameDisplay->id;

  frameChanged = prevId != currId;
  this->_frameCounter++;

  if(frameChanged)
    this->_frameDisplay->frame->OnFrameEntry();
  
  return frameChanged;
}