#include "MotorAxis.h"
#include <QDebug>

MotorAxis::MotorAxis()
{
    defined = false;
    motor_mod = 0;
    motor_mod_axis = 0;
    motor_mod_type = 0;
    encoder_mod = 0;
    encoder_mod_axis = 0;
    encoder_mod_type = 0;

    position  = 0.0;
    target    = 0.0;     // overloaded double to set position/go to target/etc
    increment = 1.0;
    targetstate = false; // utility boolean for enable/disable, etc

    statetimer   = 0;
    eligible     = false;
    state        = MOTOR_STATE_IDLE;
    newtarget    = false;
    newincrement = false;
    newposition  = false;
    newaxisenable= false;
    writeconfig  = false;
    request_stop = 0;
}

bool MotorAxis::Configure()
{
    bool retn;
    int index;

    index = parms->motax;

    retn = pModule->Configure(index, parms);
    return retn;
}


bool MotorAxis::GetPosition(double * pos)
{
    bool retn;
    int index;

    index = parms->motax;
    retn = pModule->GetPosition(index, pos);
    //qDebug() << "Get Position M["<<index<<"] = "<<*pos;
    position = * pos;
    return retn;
}

bool MotorAxis::SetPosition(double pos)
{
    bool retn;
    int index;
    index = parms->motax;

    //qDebug() << "MotorAxis::SetPosition["<<parms->num << "] = " << QString::number(pos);
    retn = pModule->SetPosition(index, pos);
    return retn;
}

bool MotorAxis::GoToDestination(double destination)
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->GoToDestination(index, destination);
    parms->moving = true; // Set it here

    return retn;
}

bool MotorAxis::GoHome(bool movepositive)
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->GoHome(index, movepositive);
    parms->moving = true;

    return retn;
}

bool MotorAxis::Oscillate(double center, double halfamp)
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->Oscillate(index, center, halfamp);
    parms->moving = true;

    return retn;
}

bool MotorAxis::EnableAxis(bool enable)
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->EnableAxis(index, enable);

    return retn;
}

bool MotorAxis::EnableLimits(bool enable)
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->EnableLimits(index, enable);

    return retn;
}

bool MotorAxis::IsMoving()
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->IsMoving(index);
    //qDebug() << "MotorAxis::IsMoving ["<<index<<"] = "<<retn;

    return retn;
}

bool MotorAxis::Stop()
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->Stop(index);

    return retn;
}

bool MotorAxis::GetAxisLimitStatus(bool *onposlimit, bool *onneglimit)
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->GetAxisLimitStatus(index,onposlimit,onneglimit);
    parms->poslimit = onposlimit;
    parms->neglimit = onneglimit;

    return retn;
}

bool MotorAxis::GetAxisEnableStatus()
{
    bool retn;
    int index;
    index = parms->motax;

    retn = pModule->GetAxisEnableStatus(index);
    parms->axis_enabled = retn;

    return retn;
}

bool MotorAxis::GetAxisFaultStatus()
{
    int index;
    index = parms->motax;

    parms->fault = pModule->GetAxisFaultStatus(index);

    return parms->fault;
}
