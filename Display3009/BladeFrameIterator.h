#ifndef BLADE_FRAME_ITERATOR_H
#define BLADE_FRAME_ITERATOR_H

#include "Constants.h"
#include "BladeFrame.h"
#include "FS.h"


struct BladeFrameNode{
  int id;
  
  BladeFrame *frame;
  BladeFrameNode *prev, *next;
};

class BladeFrameIterator{
  public:

    BladeFrameIterator();
    BladeFrameIterator(FILE *animation);

    void Destroy();

    void SetFile(FILE *animation);

    BladeFrame *GetFrame();

    bool Step();
    bool ForceStep();

  private:

    BladeFrameNode *_frameDisplay;
    long _lastFrameUpdate, _frameWait = 1000/CRENDER::FRAME_RATE;
    double _frameRate = CRENDER::FRAME_RATE;

    FILE *_animation = NULL;

};

#endif