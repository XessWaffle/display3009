#include "AngleSet.h"

AngleSet::AngleSet() {
  this->_divisions = 360;
  this->Initialize();
}

AngleSet::AngleSet(int divisions){
  this->_divisions = divisions;
  this->Initialize();
}

void AngleSet::AddCallback(double theta, void (*onAngleReached)()){
  double upper = theta + noise, lower = theta - noise;
  unsigned int index = (unsigned int) (this->_divisions * theta / (2 * PI));

  struct AngularCallback *last = this->_set[index];

  while(last->next != NULL) last = last->next;

  last->next = (AngularCallback*) malloc(sizeof(AngularCallback));
  last->next->theta = theta;
  last->next->onAngleReached = onAngleReached;
  last->next->next = NULL;
  last->next->prev = last;
}

void AngleSet::RemoveCallback(double theta, double noise){
  struct AngularCallback *remove = this->GetCallback(theta, noise), *next = remove->next;

  while(true){
    free(remove);
    remove = next;
    if(remove == NULL) break;
    next = remove->next;
  }

}

AngularCallback* AngleSet::GetCallback(double theta, double noise){

  double upper = theta + noise, lower = theta - noise;
  unsigned int index = (unsigned int) (this->_divisions * theta / (2 * PI));

  struct AngularCallback *head = this->_set[index], *iter = head, *requested = NULL;

  while(iter != NULL){
    
    struct AngularCallback *prev = iter->prev, *next = iter->next;

    if(iter->theta < upper && iter->theta > lower){
      iter->last = NULL;
      if(requested == NULL){
        requested = iter;
      } else {
        requested->next = iter;
        iter->prev = requested;
      }
      
      if(prev != NULL) prev->next = next;
      if(next != NULL) next->prev = prev;
    }

    iter = iter->next;
  }

  return requested;
}

void AngleSet::Initialize(){
  this->_set = (AngularCallback*) malloc(_divisions * sizeof(struct AngularCallback));

  for(int i = 0; i < _divisions; i++){
    this->_set[i]->theta = 0.0;
    this->_set[i]->callback = NULL;
    this->_set[i]->next = NULL;
    this->_set[i]->prev = NULL;
  }

}