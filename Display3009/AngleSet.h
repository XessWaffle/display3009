#ifndef ANGLE_SET_H
#define ANGLE_SET_H

#include <Arduino.h>

typedef void *(*callback)();

struct AngularCallback{
  double theta = 0.0;
  callback onAngleReached = NULL;
  
  struct AngularCallback *next, *prev;
};


class AngleSet{
  public:
    AngleSet();
    AngleSet(int divisions);

    void AddCallback(double theta, void* (*onAngleReached)());
    void RemoveCallback(double theta, double noise);
    AngularCallback* GetCallback(double theta, double noise);

  protected:
    void Initialize();

  private:
    AngularCallback *_set;
    int _divisions;
};

#endif

