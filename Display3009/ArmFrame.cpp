#include "ArmFrame.h"


ArmFrame::ArmFrame(){

}

ArmFrame::ArmFrame(int numLeds){
  this->_numLeds = numLeds;
  this->_ledFrame = (CRGB*) malloc(numLeds * sizeof(CRGB));

  for(int i = 0; i < numLeds; i++){
    this->_ledFrame[i] = CRGB::Black;
  }
}

void ArmFrame::SetLED(int led, CRGB color){
  if(led >= 0 && led < this->_numLeds){
    this->_ledFrame[led] = color;
  }
}

void ArmFrame::Destroy(){
  free(this->_ledFrame);
}

void ArmFrame::Trigger(struct CRGB *mod){

  for(int i = 0; i < this->_numLeds; i++){ 
    mod[i] = this->_ledFrame[i];   
  }

}