#include "ArmFrame.h"


ArmFrame::ArmFrame(){

}

ArmFrame::ArmFrame(int numLeds){
  this->_numLeds = numLeds;
  this->_leds = (struct CRGB*) malloc(sizeof(struct CRGB) * numLeds);
  this->Reset();
}

void ArmFrame::SetLED(int led, CRGB color){
  if(led >= 0 && led < this->_numLeds)
    this->_leds[led] = color;
}

void ArmFrame::Destroy(){
  free(this->_leds);
}

void ArmFrame::Reset(){
  for(int i = 0; i < this->_numLeds; i++)
    this->_leds[i] = CRGB::Black;
}

void ArmFrame::Trigger(struct CRGB *mod){
  for(int i = 0; i < this->_numLeds; i++){
    mod[i] = this->_leds[i];
  }
}