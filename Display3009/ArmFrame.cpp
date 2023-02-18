#include "ArmFrame.h"


ArmFrame::ArmFrame(){

}

ArmFrame::ArmFrame(int numLeds){
  this->_numLeds = numLeds;
}

void ArmFrame::SetLED(int led, CRGB color){
  if(led >= 0 && led < this->_numLeds){
    struct CRGBNode* node = (struct CRGBNode*) malloc(sizeof(struct CRGBNode)), *head = this->_root;
    node->color = color;
    node->led = led;
    node->next = NULL;
    node->prev = NULL;

    bool inserted = false;

    if(head == NULL){
      inserted = true;
      this->_root = node;
    } else {
      while(head != NULL){
        if(head->led == led){
          inserted = true;
          free(node);
          head->color = color;
        }

        if(head->led > led){
          inserted = true;
          struct CRGBNode *prev = head->prev;
          if(prev != NULL) prev->next = node;
          node->next = head;
          node->prev = prev;
          head->prev = node;
        }

        if(inserted)
          break;
        
        head = head->next;
      }
    }

    if(!inserted){
      head->next = node;
      node->prev = head;
    }

  }
}

void ArmFrame::Destroy(){

  if(this->_root == NULL) return;

  do{
    CRGBNode *remove = this->_root;
    free(remove);
    this->_root = this->_root->next;
  } while(this->_root != NULL);

}

void ArmFrame::Trigger(struct CRGB *mod){
  CRGBNode *head = this->_root;

  while(head != NULL){
    mod[head->led] = head->color;
    head = head->next;
  }
}