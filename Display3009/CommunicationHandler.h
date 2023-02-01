#ifndef COMMUNICATION_HANDLER_H
#define COMMUNICATION_HANDLER_H

#include "Constants.h"

struct InstructionNode{
  uint8_t instByte;
  void (*handler)(WiFiClient) = NULL;

  InstructionNode *next;
}


class CommunicationHandler{
  public:
    CommunicationHandler();

    void SetHandler(uint8_t inst, void (*handler)(WiFiClient));

    void OnRefresh();
    void OnDisconnect();

    void Step();
  
  private:
    void SendInstruction(const char* instStr, uint8_t inst);
    void Connect();

  private:
    WiFiClient _client;
    InstructionNode *_head, *_last;
};

#endif