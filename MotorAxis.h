#ifndef MOTORAXIS_H
#define MOTORAXIS_H

#include <QObject>
#include "MotorParameters.h"
#include "VMEModule.h"
#include "OMS_MAXv.h"

//
// We really need a generalized VMEMotor class but we'll shortcut that for the
// moment as we're retiring the VME58s
//

class MotorAxis
{
public:
    MotorAxis();

    bool defined;
    QString label;
    int motor_mod;
    int motor_mod_axis;
    int motor_mod_type;
    int encoder_mod;
    int encoder_mod_axis;
    int encoder_mod_type;

    double position;
    double target;
    double increment;
    bool   targetstate;

    long statetimer;
    bool eligible;
    int state;
    bool newtarget;
    bool newincrement;
    bool newposition;
    bool newaxisenable;
    bool writeconfig;
    bool request_stop;

    MotorParameters * parms;
    OMS_MAXv * pModule;

    bool Configure();
    bool GetPosition(double * position);
    bool SetPosition(double position);
    bool GoToDestination(double destination);
    bool GoHome(bool movepositive);
    bool Oscillate(double center, double halfamp);
    bool EnableAxis(bool enable);
    bool EnableLimits(bool enable);
    bool IsMoving();
    bool Stop();
    bool GetAxisLimitStatus(bool *onposlimit, bool *onneglimit);
    bool GetAxisEnableStatus();
    bool GetAxisFaultStatus();
};

// Define states
#define MOTOR_STATE_IDLE         1000
#define MOTOR_STATE_NEWPARAMETER 1001
#define MOTOR_STATE_NEWTARGET    1002
#define MOTOR_STATE_GOTARGET     1003
#define MOTOR_STATE_INMOTION     1004
#define MOTOR_STATE_ENDMOTION    1005
#define MOTOR_STATE_READPOS      1010
#define MOTOR_STATE_UPDATE       1011

#endif // MOTORAXIS_H
