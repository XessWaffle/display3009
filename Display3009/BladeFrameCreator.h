#ifndef BLADE_FRAME_CREATOR_H
#define BLADE_FRAME_CREATOR_H

class BladeFrameCreator{
  public:
    BladeFrameCreator();

    void StageFrame();
    void StageArm(double theta);
    void CommitArm();
    void CommitFrame();

    BladeFrame *GetFrame();


  private:
    BladeFrame *_stagedFrame = NULL;
    ArmFrame *_stagedArm = NULL;
};


#endif