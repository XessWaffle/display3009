#include "CommunicationHandler.h"

#include "Constants.h"

CommunicationHandler::CommunicationHandler(){

}

void CommunicationHandler::SetHandler(uint8_t inst, void (*handler)(InstructionNode*), int requiredBytes){
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
  this->SendInstruction("copy-ar", CCOMMS::COPY_ARM);
  this->SendInstruction("led", CCOMMS::SET_LED);
  this->SendInstruction("commit-ar", CCOMMS::COMMIT_ARM);
  this->SendInstruction("commit-fr", CCOMMS::COMMIT_FRAME);
}

void CommunicationHandler::OnDisconnect(){
  this->_client.stop();
}

void CommunicationHandler::Handle(){
  if(this->_headInst != NULL){
    HandlerNode *instHandler = this->GetInstructionHandler(this->_headInst->instByte);
    if(instHandler != NULL)
      (*(instHandler->handler))(this->_headInst);

    InstructionNode *remove = this->_headInst;
    this->_headInst = this->_headInst->next;
    free(remove);
  }
}

void CommunicationHandler::Populate(){
  if(!this->_client.connected())
    this->Connect();

  if(this->_client.available()){
    uint8_t byte = this->_client.read();

    if(_stagedInst == NULL){
      bool standardInst = false;

      if(byte == CCOMMS::REFRESH) {
        this->OnRefresh();
        standardInst = true;
      } else if(byte == CCOMMS::DISCONNECT) {
        this->OnDisconnect();
        standardInst = true;
      }

      if(standardInst) return;

      _stagedInst = (InstructionNode*) malloc(sizeof(InstructionNode));
      _stagedInst->next = NULL;
      _stagedInst->client = &(this->_client);

      this->_stagedHandler = this->GetInstructionHandler(byte);

      if(this->_stagedHandler != NULL){
        _stagedInst->byteCounter = this->_stagedHandler->bytes;
        _stagedInst->instByte = this->_stagedHandler->instByte;
      } else {
        free(_stagedInst);
        _stagedInst = NULL;
      }

    } else {
      _stagedInst->buff[this->_stagedHandler->bytes - _stagedInst->byteCounter] = byte;
      _stagedInst->byteCounter--;
    }

    if(_stagedInst->byteCounter == 0){
      if(this->_headInst == NULL){
        this->_headInst = _stagedInst;
        this->_lastInst = _stagedInst;
      } else {
        this->_lastInst->next = _stagedInst;
        this->_lastInst = _stagedInst;
      }
      _stagedInst = NULL;
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

HandlerNode *CommunicationHandler::GetInstructionHandler(uint8_t instByte){
  HandlerNode *head = this->_head;
  while(head != NULL){
    if(head->instByte == instByte){
      return head;
    }
    head = head->next;
  }
  
  return NULL;
}
