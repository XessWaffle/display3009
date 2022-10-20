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
    remove->frame->Destroy();
    head = head->next;
    free(remove);
    remove = head;
    removed++;
  }

}


void BladeFrameIterator::AddFrame(BladeFrame* frame){
  BladeFrameNode *next = (BladeFrameNode*) malloc(sizeof(BladeFrameNode)), *prev = NULL;

  next->id = this->_frames;
  next->frame = frame;
  next->next = NULL;
  next->prev = NULL;

  int insert = 0;

  if(this->_frameSet == NULL){
    this->_frameSet = next;
    this->_frameDisplay = next;

    next->prev = next;
    next->next = next;
  } else {
    prev = this->_frameSet->prev;
    prev->next = next;
    next->prev = prev;
    next->next = this->_frameSet;
    this->_frameSet->prev = next;
  }

  this->_frames++;

}

BladeFrame *BladeFrameIterator::GetFrame(){
  return this->_frameDisplay->frame;
}

void BladeFrameIterator::RemoveFrame(int id){
  BladeFrameNode *head = this->_frameSet;
  int queried = 0;

  if(head == NULL) return;

  bool removed = false;
  
  while(queried < this->_frames){
    if(id == head->id){
      BladeFrameNode *remove = head;
      head->prev->next = head->next;
      head->next->prev = head->prev;
      head->frame->Destroy();
      head = head->next;
      free(remove);
      this->_frames--;
      removed = true;
    }

    if(removed){
      head->id--;
    }

    queried++;
    head = head->next;
  }
}

void BladeFrameIterator::SetFrameRate(double rate){
  this->_frameRate = rate;
  this->_frameWait = 1000/rate;
}

bool BladeFrameIterator::Step(){
  long currentTime = millis();

  bool frameChanged = false;

  if(currentTime - this->_lastFrameUpdate > this->_frameWait){

    int prevId = this->_frameDisplay->id;

    if(this->_frameCounter >= this->_frames){
      this->_forward = !this->_forward;
      this->_frameCounter = 0;
    }
  
    if(this->_type == animationType::LOOP){
      this->_frameDisplay = this->_frameDisplay->next;
    } else if(this->_type == animationType::REWIND){
      this->_frameDisplay = this->_forward ? this->_frameDisplay->next : this->_frameDisplay->prev;
    }

    int currId = this->_frameDisplay->id;

    frameChanged = prevId != currId;
    this->_frameCounter++;
    this->_lastFrameUpdate = currentTime;
  }
  return frameChanged;
} 