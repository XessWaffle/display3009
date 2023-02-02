#ifndef COMMUNICATION_HANDLER_H
#define COMMUNICATION_HANDLER_H

#include <WiFi.h>

struct HandlerNode{
  uint8_t instByte;
  int bytes;
  void (*handler)(WiFiClient*) = NULL;

  HandlerNode *next;
};

struct InstructionNode{
  uint8_t instByte;
  uint8_t buff[30];
  int size = 0, byteCounter = 0;
  WiFiClient *client;

  InstructionNode *next;
}

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

  private:
    WiFiClient _client;
    InstructionNode *_headInst, *_lastInst, *_stagedInst = NULL;
    HandlerNode *_head, *_last;
    SemaphoreHandle_t _dataMutex = xSemaphoreCreateMutex();
};

#endif