#include "ArmFrame.h"
#include <Arduino.h>

ArmFrame::ArmFrame(){

}

ArmFrame::ArmFrame(frameType type, int numLeds){
  this->_type = type;
  this->_numLeds = numLeds;
  this->_ledFrame = (CRGB*) malloc(numLeds * sizeof(CRGB));
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

  FastLED.show();

  if(this->_type == frameType::FLASH){
    for(int i = 0; i < this->_numLeds; i++){
      mod[i] = CRGB::Black;
    }

    FastLED.show();
  }


}