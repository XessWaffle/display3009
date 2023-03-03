#ifndef FRAME_DATA_ALLOCATOR_H
#define FRAME_DATA_ALLOCATOR_H

#include "Constants.h"
#include "ArmFrame.h"
#include "BladeFrame.h"

class FrameDataAllocator{
    public:
        FrameDataAllocator();

        ArmFrame *CreateArmFrame();
        BladeFrame *CreateBladeFrame();

        int AllocatedDestructibles();

        bool Destroy(Destructible *frame);

        void Refresh();     

    private:
        bool Destroy(int index);

    private:
        Destructible *_data[CALLOC::ALLOCATABLE_DESTRUCTIBLES];
        int _manageIndex;

        int _allocated;
};

#endif