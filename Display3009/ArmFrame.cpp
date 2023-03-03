#include "ArmFrame.h"


ArmFrame::ArmFrame(){
  this->Reset();
}

void ArmFrame::SetLED(int led, CRGB color){
  if(led >= 0 && led < CRENDER::NUM_LEDS)
    this->_leds[led] = color;
}

void ArmFrame::Destroy(){}

void ArmFrame::Reset(){
  for(int i = 0; i < CRENDER::NUM_LEDS; i++)
    this->_leds[i] = CRGB::Black;
}

void ArmFrame::Trigger(struct CRGB *mod){
  for(int i = 0; i < CRENDER::NUM_LEDS; i++){
    mod[i] = this->_leds[i];
  }
}