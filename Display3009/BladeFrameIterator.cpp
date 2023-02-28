#include "BladeFrameIterator.h"

BladeFrameIterator::BladeFrameIterator(){
  this->_type = animationType::LOOP;
  this->_frames = 0;
  this->_frameSet = NULL;
  this->_frameDisplay = NULL;
  this->_lastFrameUpdate = millis();
}

BladeFrameIterator::BladeFrameIterator(animationType type){
  this->_frameSet = NULL;
  this->_frameDisplay = NULL;
  this->_type = type;
  this->_frames = 0;
  this->_lastFrameUpdate = millis();
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


void BladeFrameIterator::AddFrame(BladeFrame* frame){
  BladeFrameNode *frameNode = (BladeFrameNode*) malloc(sizeof(BladeFrameNode)), *prev = NULL;

  frameNode->id = this->_frames;
  frameNode->frame = frame;
  frameNode->next = NULL;
  frameNode->prev = NULL;

  int insert = 0;

  if(this->_frameSet == NULL){
    this->_frameSet = frameNode;
    this->_frameDisplay = frameNode;
    this->_frameLast = frameNode;

    if(this->_type != animationType::STREAM){
      frameNode->prev = frameNode;
      frameNode->next = frameNode;
    }
  } else {
    if(this->_type != animationType::STREAM){
      prev = this->_frameSet->prev;
      prev->next = frameNode;
      frameNode->prev = prev;
      frameNode->next = this->_frameSet;
      this->_frameSet->prev = frameNode;
    } else {
      this->_frameLast->next = frameNode;
      frameNode->prev = this->_frameLast;
      this->_frameLast = frameNode;
    }
  }

  this->_frames++;

}

BladeFrame *BladeFrameIterator::GetFrame(){
  if(this->_frames > 0)
    return this->_frameDisplay->frame;
  return NULL;
}


void BladeFrameIterator::SetFrameRate(double rate){
  this->_frameRate = rate;
  this->_frameWait = 1000/rate;
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
        remove->frame->Mark();
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