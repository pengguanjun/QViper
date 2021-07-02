#ifndef MOTORPARAMETERS_H
#define MOTORPARAMETERS_H

#include <QObject>

#define NAXIS 50 // Maximum number of axes

class MotorParameters : public QObject
{
public:
    explicit MotorParameters(QObject *parent = nullptr);
    ~MotorParameters();

    int num;
    bool defined;
    char label[40];

    int motmod;
    int motax;
    int encmod;
    int encax;

    int MotorStepsPerRevolution;    // Drive resolution
    int EncoderStepsPerRevolution;
    int EncoderZero;                // Zero for absolute encoders
    int EncoderDirection;           // Normal / reversed
    int deadband;

    double dscale;     /* Distance scale units/revolution */
    double bscale;     /* Base velocity scale units/revolution-sec */
    double vscale;     /* Velocity scale units/revolution-sec */
    double ascale;     /* Acceleration scale units/revolution-sec^2 */
    double kp;         /* Proportional gain */
    double ki;         /* Integral gain */
    double kd;         /* Differential gain */
    double ka;         /* Acceleration feedforward */

    bool is_servo;
    bool use_encoder;
    bool stalldetection;
    bool positionmaintenance;
    bool axis_enabled;
    bool limits_enabled;
    bool enable_high;
    bool limit_high;
    bool home_high;     /* Home parity 1=High true 0=Low true */
    bool home_encoder;  /* Use encoder index line to home 0=switch, 1=encoder */

    int acceleration;  /* Acceleration in motor pulses/revolution-sec^2 */
    int basevelocity;
    int topvelocity;

    /* Status */
    int MotorRawPosition;
    int EncoderRawPosition;

    double position;
    double increment;
    double target;

    bool poslimit;      // Positive limit hit
    bool neglimit;      // Negative limit hit

    bool athome;
    bool moving;
    bool done;
    bool fault;

    int  direction;
};

#endif // MOTORPARAMETERS_H
