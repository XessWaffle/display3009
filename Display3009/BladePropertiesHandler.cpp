#include "BladePropertiesHandler.h"

#include "Constants.h"

BladePropertiesHandler::BladePropertiesHandler() : CommunicationHandler(CCOMMS::PROPS_ID){}

void BladePropertiesHandler::OnRefresh(){
    this->_client.write(CCOMMS::PROP_FCNS);
    this->SendInstruction("throttle", CCOMMS::THROTTLE);
    this->SendInstruction("start", CCOMMS::START);
    this->SendInstruction("stop", CCOMMS::STOP);
    this->SendInstruction("test", CCOMMS::TEST);
    this->SendInstruction("state", CCOMMS::STATE);
    this->SendInstruction("anim", CCOMMS::ANIMATION);
    this->SendInstruction("fps", CCOMMS::FPS);
    this->SendInstruction("mult", CCOMMS::MULTIPLIER);
}


void BladePropertiesHandler::Populate(){
    CommunicationHandler::Populate();

    if(this->StagedInstructions() >= 1)
        this->Handle();
}