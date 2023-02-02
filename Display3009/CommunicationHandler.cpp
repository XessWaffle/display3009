#include "CommunicationHandler.h"

#include "Constants.h"
#include <Arduino.h>

CommunicationHandler::CommunicationHandler(){

}

void CommunicationHandler::SetHandler(uint8_t inst, void (*handler)(WiFiClient*), int requiredBytes){
  HandlerNode *instNode = (HandlerNode*) malloc(sizeof(HandlerNode));
  instNode->handler = handler;
  instNode->instByte = inst;
  instNode->bytes = requiredBytes;
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
  this->_client.write(CCOMMS::NUM_INST);
  this->SendInstruction("throttle", CCOMMS::THROTTLE);
  this->SendInstruction("start", CCOMMS::START);
  this->SendInstruction("stop", CCOMMS::STOP);
  this->SendInstruction("test", CCOMMS::TEST);
  this->SendInstruction("state", CCOMMS::STATE);
  this->SendInstruction("anim", CCOMMS::ANIMATION);
  this->SendInstruction("fps", CCOMMS::FPS);
  this->SendInstruction("mult", CCOMMS::MULTIPLIER);
  this->SendInstruction("stage-fr", CCOMMS::STAGE_FRAME);
  this->SendInstruction("stage-ar", CCOMMS::STAGE_ARM);
  this->SendInstruction("led", CCOMMS::SET_LED);
  this->SendInstruction("commit-ar", CCOMMS::COMMIT_ARM);
  this->SendInstruction("commit-fr", CCOMMS::COMMIT_FRAME);
}

void CommunicationHandler::OnDisconnect(){
  this->_client.stop();
}

void CommunicationHandler::Handle(){
  if(this->_headInst != NULL){

    HandlerNode *head = this->_head;
    while(head != NULL){
      if(head->instByte == this->_headInst->instByte){
        (*(head->handler))(&this->_headInst);
        break;     
      } 
      head = head->next;
    }

    xSemaphoreTake(this->_dataMutex, portMAX_DELAY);
    InstructionNode *remove = this->_headInst;
    this->_headInst = this->_headInst->next;
    free(remove)
    xSemaphoreGive(this->_dataMutex);
  }
}

void CommunicationHandler::Populate(){
  if(!this->_client.connected())
    this->Connect();

  if(this->_client.available()){
    uint8_t byte = this->_client.read();

    if(_stagedInst == NULL){
      _stagedInst = (InstructionNode*) malloc(sizeof(InstructionNode));
      _stagedInst->next = NULL;
      _stagedInst->client = &(this->_client);

      HandlerNode *head = this->_head;
      bool foundInst = false;
      while(head != NULL){
        if(head->instByte == byte){
          _stagedInst.byteCounter = head->bytes;
          _stagedInst.instByte = byte;
          foundInst = true;
          break;
        }
        head = head->next;
      }

      if(!foundInst){
        free(_stagedInst);
        _stagedInst = NULL;
      }
    } else {
      _stagedInst->buff[_stagedInst->size++] = byte;
      _stagedInst->byteCounter--;

      xSemaphoreTake(this->_dataMutex, portMAX_DELAY);

      if(_stagedInst->byteCounter == 0){
        if(this->_headInst == NULL){
          this->_headInst = _stagedInst;
          this->_lastInst = _stagedInst;
        } else {
          this->_lastInst->next = _stagedInst;
          this->_lastInst = _stagedInst;
          _stagedInst = NULL;
        }
      }

      xSemaphoreGive(this->_dataMutex);
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