#include "CommandParser.h"

/*
 * Run CommChannel in blocking mode
 *
 *
 */

const QString CommandParser::ERR_UNDEFINED_DEVICE    = "ERR:undefined device";
const QString CommandParser::ERR_COMMAND_UNSUPPORTED = "ERR:command not supported";
const QString CommandParser::ERR_COMMAND_NOT_FOUND   = "ERR:command not found";
const QString CommandParser::ERR_CHANNEL_NOT_FOUND   = "ERR:channel not found";
const QString CommandParser::ERR_PARAMETER_INVALID   = "ERR:parameter invalid";
const QString CommandParser::ERR_MOTOR_DISABLED      = "ERR:motor disabled";

CommandParser::CommandParser(MotorAxis * p_Axis[])
{
    int i;
    for (i=0;i<NAXIS;i++) {
        m_Axis[i] = p_Axis[i];
    }
}


const QString CommandParser::ProcessMessage(QString &command)
{
    QString retStr;
    if (command.isEmpty()) return "";

    QStringList list = command.split(QRegExp("\\s"),Qt::SkipEmptyParts);

    DeviceDictType type;
    if (s_mapStringToDeviceDictType.contains(list[0])) {
        type = s_mapStringToDeviceDictType.value(list[0]);
    } else
        return ERR_UNDEFINED_DEVICE;

    switch(type)
    {
    case DeviceDictType::motor:
        retStr = ProcessMotorCommand(&list);
        break;
    case DeviceDictType::dio:
    case DeviceDictType::scaler:
    case DeviceDictType::relay:
        return ERR_COMMAND_UNSUPPORTED;
    default:
        return ERR_UNDEFINED_DEVICE;
    }

    return retStr;
}

const QString CommandParser::ProcessMotorCommand(QStringList *pCommandAndPars)
{
    MotorCommandDictType type;
    QString verb;
    if ((*pCommandAndPars).size()<2)
        return ERR_PARAMETER_INVALID;

    verb = (*pCommandAndPars).at(1);

    if (s_mapStringToMotorCommandDictType.contains(verb)) {
        type = s_mapStringToMotorCommandDictType.value(verb);
    } else
        return ERR_COMMAND_NOT_FOUND;

    switch(type) {
    case MotorCommandDictType::stop:
        return ProcessMotorStop(pCommandAndPars);
    case MotorCommandDictType::position:
        return ProcessMotorPosition(pCommandAndPars);
    case MotorCommandDictType::move:
        return ProcessMotorMove(pCommandAndPars);
    case MotorCommandDictType::motion:
        return ProcessMotorIsMoving(pCommandAndPars);
    case MotorCommandDictType::enable:
        return ProcessMotorEnable(pCommandAndPars);
    case MotorCommandDictType::enabled:
        return ProcessMotorEnabled(pCommandAndPars);
    case MotorCommandDictType::disable:
        return ProcessMotorDisable(pCommandAndPars);
    case MotorCommandDictType::limits:
        return ProcessMotorLimits(pCommandAndPars);
    default:
        return ERR_COMMAND_NOT_FOUND;
    }

    return ""; // Should never get here
}

const QString CommandParser::ProcessMotorStop(QStringList *pCommandAndPars)
{
    if ((*pCommandAndPars).length() == 2) {
        // Stop everything
        int i;
        for (i=0;i<NAXIS;i++) {
            if (m_Axis[i]->defined) {
                m_Axis[i]->request_stop = true;
            }
        }
    } else {
        QString axis = (*pCommandAndPars).value(2);
        int index = axis.toInt();
        if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;

        if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;
        m_Axis[index]->request_stop = true;
    }
    return "OK:stop requested";
}

// Format:
// motor position <axis>
// motor position <axis> <position>
//
// Ignore parameters past 2
const QString CommandParser::ProcessMotorPosition(QStringList *pCommandAndPars)
{
    double position, target;
    if ((*pCommandAndPars).size() < 3) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    if ((*pCommandAndPars).size() == 3) {
        position = m_Axis[index]->position;
        // return current position
        return "OK: " + QString::number(position,'f',4);
    } else {
        target = (*pCommandAndPars).value(3).toDouble();
        m_Axis[index]->target = target;
        m_Axis[index]->newposition = true;
        return "OK: set axis " + QString::number(index) + " to " + QString::number(target,'f',4);
        //m_Axis[index]->SetPosition()
    }

    return "OK: did the thing";
}

// Format:
// motor move <axis> <position>
//
// Ignore parameters past 2
const QString CommandParser::ProcessMotorMove(QStringList *pCommandAndPars)
{
    double target;
    if ((*pCommandAndPars).size() < 4) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;

    target = (*pCommandAndPars).value(3).toDouble();
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    // Now send command to state machine
    m_Axis[index]->target = target;
    m_Axis[index]->newtarget = true;

    return "OK: moving axis " + QString::number(index) + " to " + QString::number(target);
}

// Format:
// motor enable <axis>
//
// Ignore parameters past 2
const QString CommandParser::ProcessMotorEnable(QStringList *pCommandAndPars)
{
    double target;
    if ((*pCommandAndPars).size() < 3) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    // Now send command to state machine
    m_Axis[index]->targetstate = true;
    m_Axis[index]->newaxisenable = true;

    return "OK: enabling axis " + QString::number(index);
}

// Format:
// motor enabled <axis>
//
// Ignore param
const QString CommandParser::ProcessMotorEnabled(QStringList *pCommandAndPars)
{
    int retn;
    if ((*pCommandAndPars).size() < 3) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    retn = (m_Axis[index]->parms->axis_enabled) ? 1 : 0;
    return "OK:" + QString::number(retn);
}

// Format:
// motor disable <axis>
//
// Ignore parameters past 2
const QString CommandParser::ProcessMotorDisable(QStringList *pCommandAndPars)
{
    double target;
    if ((*pCommandAndPars).size() < 3) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    // Now send command to state machine
    m_Axis[index]->targetstate = false;
    m_Axis[index]->newaxisenable = true;

    return "OK: disabling axis " + QString::number(index);
}

// Format:
// motor motion <axis>
//
// Ignore parameters past 2
const QString CommandParser::ProcessMotorIsMoving(QStringList *pCommandAndPars)
{

    if ((*pCommandAndPars).size() < 3) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    if(m_Axis[index]->parms->moving) return "OK:1";

    return "OK:0"; // Default reply is no motion
}

const QString CommandParser::ProcessMotorLimits(QStringList *pCommandAndPars)
{
    int limval;
    if ((*pCommandAndPars).size() < 3) return ERR_PARAMETER_INVALID;

    int index  = (*pCommandAndPars).value(2).toInt();
    if ((index < 0) || (index >= NAXIS)) return ERR_PARAMETER_INVALID;
    if (!m_Axis[index]->defined) return ERR_PARAMETER_INVALID;

    limval = 0;
    if (m_Axis[index]->parms->poslimit) limval += 2;
    if (m_Axis[index]->parms->neglimit) limval += 1;

    return "OK:" + QString::number(limval); // Default reply is no limits hit;
}

