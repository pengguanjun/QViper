#include "MotorParameters.h"
#include <QDebug>

MotorParameters::MotorParameters(QObject *parent) : QObject(parent)
{
    defined = false;
    sprintf(label," ");
    num    = -1;
    motmod = -1;
    motax  = -1;
    encmod = -1;
    encax  = -1;
    MotorStepsPerRevolution   = 25000;
    EncoderStepsPerRevolution = 4096;
    EncoderZero               = 0;
    EncoderDirection          = 1; // Normal direction
    deadband = 5;

    MotorRawPosition = 0;
    EncoderRawPosition = 0;
    position = 0;

    dscale = 1.0;
    bscale = 1.0;
    vscale = 2.0;
    ascale = 1.0;
    kp = 32.0;
    ki = 1.0;
    kd = 100.0;
    ka = 2.0;

    acceleration = 25000;
    topvelocity  = 25000;
    basevelocity = 10000;

    enable_high    = true;
    is_servo       = false;
    use_encoder    = false;
    stalldetection = false;
    positionmaintenance = false;
    axis_enabled   = false;
    limits_enabled = false;
    limit_high     = false;
    enable_high    = true;
    home_high      = true;

    athome = false;
    poslimit = false;
    neglimit = false;
    moving = false;
    fault = false;
    direction = 1;

}

MotorParameters::~MotorParameters() {
    qDebug() << "MotorParameters destructor called";
}
