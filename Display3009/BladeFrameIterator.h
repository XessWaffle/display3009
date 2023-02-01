#ifndef BLADE_FRAME_ITERATOR_H
#define BLADE_FRAME_ITERATOR_H

#define DEFAULT_FRAME_RATE 30.0

#include "BladeFrame.h"

struct BladeFrameNode{
  int id;
  
  BladeFrame *frame;
  BladeFrameNode *prev, *next;
};

class BladeFrameIterator{
  public:

    typedef enum {LOOP, REWIND, STREAM} animationType;

    BladeFrameIterator();
    BladeFrameIterator(animationType type);

    void Destroy();

    void AddFrame(BladeFrame* frame);
    BladeFrame *GetFrame();

    void SetFrameRate(double rate);

    bool Step();

  private:

    BladeFrameNode *_frameSet, *_frameDisplay;
    int _frames, _frameCounter;
    long _lastFrameUpdate, _frameWait = 1000/DEFAULT_FRAME_RATE;
    double _frameRate = DEFAULT_FRAME_RATE;
    animationType _type;
    bool _forward = true;

};

#endif