#ifndef FRAME_TRANSFER_HANDLER_H
#define FRAME_TRANSFER_HANDLER_H

#include "CommunicationHandler.h"

class FrameTransferHandler : public CommunicationHandler{
    public:
        FrameTransferHandler();
        FrameTransferHandler(uint8_t id);

        void OnRefresh();

        void Populate();
        void Flush();
};

#endif