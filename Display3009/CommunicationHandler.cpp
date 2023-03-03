#include "CommunicationHandler.h"

#include "Constants.h"

CommunicationHandler::CommunicationHandler(){

}

CommunicationHandler::CommunicationHandler(uint8_t id){
  this->_id = id;
}

void CommunicationHandler::SetHandler(uint8_t inst, void (*handler)(InstructionNode*), uint8_t requiredBytes){
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
}

void CommunicationHandler::OnDisconnect(){
  this->_client.stop();
}

void CommunicationHandler::Handle(int instructions){
  for(int i = 0; i < instructions; i++){
    if(this->_headInst != NULL){

      HandlerNode *instHandler = this->GetInstructionHandler(this->_headInst->instByte);
      if(instHandler != NULL)
        (*(instHandler->handler))(this->_headInst);

      InstructionNode *remove = this->_headInst;
      this->_headInst = this->_headInst->next;
      free(remove);
      _staged--;
    } else {
      return;
    }
  }
}

bool CommunicationHandler::Populate(){
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

      if(standardInst) return true;

      _stagedInst = (InstructionNode*) malloc(sizeof(InstructionNode));
      _stagedInst->next = NULL;

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
      _staged++;
    }

    return true;
  }

  return false;
}

WiFiClient *CommunicationHandler::Client(){
  return &(this->_client);
}

int CommunicationHandler::StagedInstructions(){
  return _staged;
}


uint8_t CommunicationHandler::PeekLastInstruction(){
  if(_staged > 0)
    return this->_lastInst->instByte;
  
  return (uint8_t) 0xFF;
}


void CommunicationHandler::SendInstruction(const char* instStr, uint8_t inst){
  this->_client.print(instStr);
  this->_client.write(CCOMMS::MESSAGE_PAUSE);
  this->_client.write(inst);
}

void CommunicationHandler::Connect(){
  if (this->_client.connect(CCOMMS::XESSAMD, CCOMMS::PORT)) {
    this->_client.write((uint8_t) this->_id);
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
