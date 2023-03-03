#include "Constants.h"
#include "FrameDataAllocator.h"

FrameDataAllocator::FrameDataAllocator(){
    for(int i = 0; i < CALLOC::ALLOCATABLE_DESTRUCTIBLES; i++)
        this->_data[i] = NULL;
    
    this->_manageIndex = 0;
    this->_allocated = 0;
}

ArmFrame *FrameDataAllocator::CreateArmFrame(){
    int i = 0;
    while(this->_data[i] != NULL){
        i++;
        if(i >= CALLOC::ALLOCATABLE_DESTRUCTIBLES)
          return NULL;
    };

    if(i < CALLOC::ALLOCATABLE_DESTRUCTIBLES){
        this->_data[i] = (Destructible*)(new ArmFrame());
        this->_allocated++;
        return (ArmFrame*) this->_data[i];
    }

    return NULL;
}

BladeFrame *FrameDataAllocator::CreateBladeFrame(){
    int i = 0;
    while(this->_data[i] != NULL){
        i++;
        if(i >= CALLOC::ALLOCATABLE_DESTRUCTIBLES)
          return NULL;
    };

    this->_data[i] = (Destructible*)(new BladeFrame());
    this->_allocated++;
    return (BladeFrame*) this->_data[i];
}


int FrameDataAllocator::AllocatedDestructibles(){
    return this->_allocated;
}


bool FrameDataAllocator::Destroy(Destructible *frame){
    int i = 0;
    while(this->_data[i] != frame){
        i++;
        if(i >= CALLOC::ALLOCATABLE_DESTRUCTIBLES)
          return false;
    };

    return this->Destroy(i);
}

bool FrameDataAllocator::Destroy(int index){
    this->_data[index]->Destroy();
    delete this->_data[index];
    this->_data[index] = NULL;
    this->_allocated--;
    return true;
}

void FrameDataAllocator::Refresh(){
    for(int i = 0; i < CALLOC::ALLOCATABLE_DESTRUCTIBLES; i++)
        if(this->_data[i] != NULL && this->_data[i]->Marked())
            this->Destroy(i);
}