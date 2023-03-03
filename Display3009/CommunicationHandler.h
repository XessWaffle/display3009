#ifndef COMMUNICATION_HANDLER_H
#define COMMUNICATION_HANDLER_H

#include <WiFi.h>

struct InstructionNode{
  uint8_t instByte;
  union{  
    uint8_t buff[12];
    int data[3] = {0,0,0};
  };
  uint8_t byteCounter = 0;

  InstructionNode *next;
};

struct HandlerNode{
  uint8_t instByte;
  uint8_t bytes;
  void (*handler)(InstructionNode*) = NULL;

  HandlerNode *next;
};

class CommunicationHandler{
  public:
    CommunicationHandler();
    CommunicationHandler(uint8_t id);

    void SetHandler(uint8_t inst, void (*handler)(InstructionNode*), uint8_t requiredBytes);

    virtual void OnRefresh();
    void OnDisconnect();

    void Handle(int instructions = 1);
    bool Populate();

    WiFiClient *Client();

    int StagedInstructions();
  
  protected:
    uint8_t PeekLastInstruction();
    void SendInstruction(const char* instStr, uint8_t inst);
    void Connect();
    HandlerNode *GetInstructionHandler(uint8_t instByte);

  protected:
    WiFiClient _client;

  private:
    InstructionNode *_headInst, *_lastInst, *_stagedInst = NULL;
    HandlerNode *_head, *_last, *_stagedHandler = NULL;
    
    uint8_t _id = 1;
    int _staged = 0;
};

#endif