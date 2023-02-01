#include "CommunicationHandler.h"

CommunicationHandler::CommunicationHandler(){
  this->Connect();
}

void CommunicationHandler::SetHandler(uint8_t inst, void (*handler)(WiFiClient)){
  InstructionNode *instNode = (InstructionNode*) malloc(sizeof(InstructionNode));
  instNode->handler = handler;
  instNode->instByte = inst;
  instNode->next = NULL;

  if(this->_head == NULL){
    this->_head = instNode;
    this->_last = instNode;
  } else {
    this->_last->next = instNode;
    this->_last = instNode;
  }
}

void CommunicationHandler::OnRefresh(){
  client.write(CCOMMS::NUM_INST);
  this->SendInstruction("throttle", CCOMMS::THROTTLE);
  this->SendInstruction("start", CCOMMS::START);
  this->SendInstruction("stop", CCOMMS::STOP);
  this->SendInstruction("test", CCOMMS::TEST);
  this->SendInstruction("state", CCOMMS::STATE);
  this->SendInstruction("anim", CCOMMS::ANIMATION);
  this->SendInstruction("fps", CCOMMS::FPS);
  this->SendInstruction("mult", CCOMMS::MULTIPLIER);
}

void CommunicationHandler::OnDisconnect(){
  this->_client.stop();
}

void CommunicationHandler::Step(){
  if(!_client.connected())
    this->Connect();

  if(client.available()){
    uint8_t byte = client.read();

    if(byte == CCOMMS::REFRESH){
      this->OnRefresh();
      return;
    } else if(byte == CCOMMS::DISCONNECT){
      this->OnDisconnect();
      return;
    }

    InstructionNode *head = this->_head;

    while(head->next != NULL)
      if(head->instByte == byte && head->handler != NULL){
        *(head->handler)(this->_client);
        break;
      }

  }


}

void CommunicationHandler::SendInstruction(const char* instStr, uint8_t inst){
  this->_client.print(instStr);
  this->_client.write(CCOMMS::MESSAGE_PAUSE);
  this->_client.write(inst);
}

void CommunicationHandler::Connect(){
  if (this->_client.connect(CCOMMS::XESSAMD, CCOMMS::PORT)) {
    this->_client.write((uint8_t) CCOMMS::ID);
  }
}