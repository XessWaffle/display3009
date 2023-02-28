#include "FrameDataAllocator.h"

FrameDataAllocator(){
    for(int i = 0; i < CALLOC::ALLOCATABLE_DESTRUCTIBLES; i++)
        this->_data[i] = NULL;
}

ArmFrame *CreateArmFrame(){
    int i = -1;
    while(i + 1 < CALLOC::ALLOCATABLE_DESTRUCTIBLES && this->_data[++i] != NULL);

    if(i < CALLOC::ALLOCATABLE_DESTRUCTIBLES){
        this->_data[i] = (Destructible*)(new ArmFrame(CRENDER::NUM_LEDS));
        return this->_data[i];
    }

    return NULL;
}


BladeFrame *CreateBladeFrame(){
    int i = -1;
    while(i + 1 < CALLOC::ALLOCATABLE_DESTRUCTIBLES && this->_data[++i] != NULL);

    if(i < CALLOC::ALLOCATABLE_DESTRUCTIBLES){
        this->_data[i] = (Destructible*)(new BladeFrame());
        return this->_data[i];
    }

    return NULL;
}

bool Destroy(Destructible *frame){
    int i = -1;
    while(i + 1 < CALLOC::ALLOCATABLE_DESTRUCTIBLES && this->_data[++i] != frame);

    if(i >= CALLOC::ALLOCATABLE_DESTRUCTIBLES)
        return false;

    this->_data[i]->Destroy();
    delete this->_data[i];
    this->_data[i] = NULL;
    return true;
}

void Refresh(){
    for(int i = 0; i < CALLOC::ALLOCATABLE_DESTRUCTIBLES; i++)
        if(this->_data[i]->Marked())
            this->Destroy(this->_data[i]);
}