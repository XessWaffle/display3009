#ifndef BLADE_PROPERTIES_HANDLER_H
#define BLADE_PROPERTIES_HANDLER_H

#include "CommunicationHandler.h"

class BladePropertiesHandler : public CommunicationHandler{
    public:
        BladePropertiesHandler();

        void OnRefresh();

        void Populate();
};

#endif