#ifndef VMEMOTOR_H
#define VMEMOTOR_H

#include "VMEModule.h"
#include "MotorParameters.h"

class VMEMotor : public VMEModule
{
public:
    VMEMotor();

    virtual bool Configure(int index, MotorParameters * mparm)=0;
    virtual bool GetPosition(int index, double * position)=0;
    virtual bool SetPosition(int index, double position)=0;
    virtual bool GoToDestination(int index, double destination)=0;
    virtual bool GoHome(int index, bool movepositive)=0;
    virtual bool Oscillate(int index, double center, double halfamp)=0;
    virtual bool EnableAxis(int index, bool enable)=0;
    virtual bool EnableLimits(int index, bool enable)=0;
    virtual bool IsMoving(int index)=0;
    virtual bool Stop(int index)=0;
    virtual bool GetAxisLimitStatus(int index, bool *onposlimit, bool *onneglimit)=0;
    virtual bool GetAxisEnableStatus(int index)=0;
    virtual bool GetAxisFaultStatus(int index)=0;
};

#endif // VMEMOTOR_H
