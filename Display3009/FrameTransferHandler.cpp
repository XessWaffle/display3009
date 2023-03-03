#include "FrameTransferHandler.h"

#include "Constants.h"

FrameTransferHandler::FrameTransferHandler(){}

FrameTransferHandler::FrameTransferHandler(uint8_t id) : CommunicationHandler(id){}

void FrameTransferHandler::OnRefresh(){
    this->_client.write(CCOMMS::FRAME_FCNS);
    this->SendInstruction("stage-fr", CCOMMS::STAGE_FRAME);
    this->SendInstruction("stage-ar", CCOMMS::STAGE_ARM);
    this->SendInstruction("led", CCOMMS::SET_LED);
    this->SendInstruction("leds", CCOMMS::SET_LEDS);
    this->SendInstruction("commit-ar", CCOMMS::COMMIT_ARM);
    this->SendInstruction("commit-fr", CCOMMS::COMMIT_FRAME);
}


void FrameTransferHandler::Populate(){
    CommunicationHandler::Populate();

    uint8_t insnByte = this->PeekLastInstruction();
    if(insnByte == CCOMMS::COMMIT_ARM || insnByte == CCOMMS::COMMIT_FRAME)
        this->Flush();

}

void FrameTransferHandler::Flush(){
    this->Handle(this->StagedInstructions());
}

