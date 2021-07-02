#include "MotorManagement.h"
#include "AppConfig.h"
#include <QProcessEnvironment>
#include <QDebug>

#define MOTOR_CACHE_FILE   "motpos.dat"

#define MOTOR_INMOTION_MS 100
#define MOTOR_STOP_MS     250
#define MOTOR_IDLE_MS     250

// Utility function to get the current time in ms
// Use for managing the state machine
long GetTickCount(void) {
    long ms;
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC,&tp);
    ms = (long) (tp.tv_sec * 1000 + tp.tv_nsec/1000000);

    return ms;
}


MotorManagement::MotorManagement(MotorAxis * p_Axis[])
{
    int i;

    for (i=0;i<NAXIS; i++) {
        m_Axis[i] = p_Axis[i];
    }
}

void MotorManagement::run() {
    int i;
    int ms;
    struct timespec wait, rem;

    qDebug() << "Starting Motor Management Thread";
    wait.tv_sec  = 0;
    wait.tv_nsec = 50000000; // 50 ms
    while(true) {
        // Determine which axes are eligible for service
        ms = GetTickCount();
        for (i=0;i<NAXIS; i++) {
            if (m_Axis[i]->defined) {
                m_Axis[i]->eligible = (ms >= m_Axis[i]->statetimer) ? 1 : 0;
            }
        }

        // Prioritize who gets service based on eligibility
        for (i=0;i<NAXIS;i++) {
            if (m_Axis[i]->eligible) {
                ProcessMotor(i);
                m_Axis[i]->eligible=false;
            }
        }
        nanosleep(&wait,&rem);
    }
}

void MotorManagement::SavePositions()
{
    QString homedir = QProcessEnvironment::systemEnvironment().value("HOME","/usr/local/QViper");
    QString cfgfile = homedir + "/" + MOTOR_CACHE_FILE;

    MC_PositionCache(cfgfile.toStdString().c_str(),m_Axis);
    return;
}

void MotorManagement::ProcessMotor(int index) {
    long cur_ms;
    double position;
    bool moving;

    MotorAxis * mptr;

    mptr = m_Axis[index];

    cur_ms = GetTickCount();

    if (mptr->request_stop) {
        mptr->Stop();
        mptr->request_stop = false; // reset
        mptr->statetimer = cur_ms + MOTOR_STOP_MS;
        mptr->state = MOTOR_STATE_ENDMOTION;
    }

    switch(mptr->state) {
    case MOTOR_STATE_IDLE:
        mptr->statetimer=cur_ms;
        if (mptr->newtarget) {
            mptr->state = MOTOR_STATE_NEWTARGET;
        } else if (mptr->writeconfig) {
            mptr->state = MOTOR_STATE_NEWPARAMETER;
        } else if (mptr->newposition) {
            mptr->state = MOTOR_STATE_NEWPARAMETER;
        } else if (mptr->newaxisenable) {
            mptr->state = MOTOR_STATE_NEWPARAMETER;
        } else {
            mptr->state = MOTOR_STATE_UPDATE;
            mptr->statetimer=cur_ms + MOTOR_IDLE_MS;
        }
        break;
    case MOTOR_STATE_NEWPARAMETER:
        if (mptr->newposition) {
            mptr->SetPosition(mptr->target);
            mptr->newposition=false;     // Reset the flag
        } else if (mptr->newaxisenable) {
            mptr->EnableAxis(mptr->targetstate);
            mptr->newaxisenable = false; // Reset the flag
        } else if (mptr->writeconfig) {
            qDebug() << "Configuring axis";
            mptr->Configure();           // Assume all of the data items in the class have been updated
            mptr->writeconfig = false;
        }
        mptr->state = MOTOR_STATE_IDLE;
        mptr->statetimer=cur_ms + MOTOR_IDLE_MS;
        break;
    case MOTOR_STATE_NEWTARGET:
        mptr->newtarget = false;
        mptr->GoToDestination(mptr->target);
        mptr->state = MOTOR_STATE_INMOTION;
        mptr->statetimer = cur_ms + MOTOR_INMOTION_MS;
        break;
    case MOTOR_STATE_INMOTION:
        moving = mptr->IsMoving();
        if (moving) {
            mptr->parms->moving = moving;
            mptr->state = MOTOR_STATE_INMOTION;
            mptr->statetimer = cur_ms + MOTOR_INMOTION_MS;
        } else {
            mptr->parms->moving = moving; // Should not need to do this, but...
            mptr->state = MOTOR_STATE_ENDMOTION;
        }
        mptr->GetPosition(&position); // Update position while moving
        mptr->position = position;
        break;
    case MOTOR_STATE_ENDMOTION:
        mptr->parms->moving = false;
        mptr->GetPosition(&position);
        mptr->position = position;
        SavePositions();
        mptr->state = MOTOR_STATE_UPDATE;
    case MOTOR_STATE_UPDATE:
        bool positivelimit, negativelimit, faulted,enabled;
        mptr->GetAxisLimitStatus(&positivelimit,&negativelimit);
        mptr->parms->poslimit = positivelimit;
        mptr->parms->neglimit = negativelimit;
        faulted = mptr->GetAxisFaultStatus();
        enabled = mptr->GetAxisEnableStatus();
        mptr->GetPosition(&position);
        mptr->position = position;
        mptr->state = MOTOR_STATE_IDLE;
        mptr->statetimer=cur_ms + MOTOR_IDLE_MS;
        break;
    }
    return;
}
