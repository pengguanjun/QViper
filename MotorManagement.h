#ifndef MOTORMANAGEMENT_H
#define MOTORMANAGEMENT_H

#include <QThread>
#include <QObject>
#include "MotorAxis.h"

class MotorManagement : public QThread
{
public:
    MotorManagement(MotorAxis * p_Axis[]);

    MotorAxis * m_Axis[NAXIS];

    void run() override;
    void ProcessMotor(int index);
    void SavePositions();
};

#endif // MOTORMANAGEMENT_H
