#ifndef COMMUNICATION_HANDLER_H
#define COMMUNICATION_HANDLER_H

#include <WiFi.h>

struct InstructionNode{
  uint8_t instByte;
  union{  
    uint8_t buff[32];
    int data[8];
  }
  int size = 0, byteCounter = 0;
  WiFiClient *client;

  InstructionNode *next;
};

struct HandlerNode{
  uint8_t instByte;
  int bytes;
  void (*handler)(InstructionNode*) = NULL;

  HandlerNode *next;
};

class CommunicationHandler{
  public:
    CommunicationHandler();

    void SetHandler(uint8_t inst, void (*handler)(InstructionNode*), int requiredBytes);

    void OnRefresh();
    void OnDisconnect();

    void Handle();
    void Populate();
  
  private:
    void SendInstruction(const char* instStr, uint8_t inst);
    void Connect();
    HandlerNode *GetInstructionHandler(uint8_t instByte);

  private:
    WiFiClient _client;
    InstructionNode *_headInst, *_lastInst, *_stagedInst = NULL;
    HandlerNode *_head, *_last;
    SemaphoreHandle_t _dataMutex = xSemaphoreCreateMutex();
};

#endif