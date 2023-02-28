#ifndef FRAME_DATA_ALLOCATOR_H
#define FRAME_DATA_ALLOCATOR_H

#include "Constants.h"

class FrameDataAllocator{
    public:
        FrameDataAllocator();

        ArmFrame *CreateArmFrame();
        BladeFrame *CreateBladeFrame();

        bool Destroy(Destuctible *frame);

        void Refresh();     

    private:
        Destructible *_data[CALLOC::ALLOCATABLE_DESTRUCTIBLES];
}

#endif