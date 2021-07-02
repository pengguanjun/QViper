#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QThread>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMap>
#include "AppDef.h"
#include "MotorAxis.h"

//#define NAXIS 50


class CommandParser : public QObject
{
    Q_OBJECT
public:
    CommandParser(MotorAxis * p_Axis[]);

private:
    char msgBuffer[256];
    char * p_msgBuffer;

    QString m_Command;
    QString m_Response;

    MotorAxis * m_Axis[NAXIS];

public:
    const QString ProcessMessage(QString& command);

    static QStringList m_DeviceDictionary;
    static QStringList m_MotorCommandDictionary;

    /**********************  Device Dictionary   ************************/
    enum class DeviceDictType
    {
        motor, scaler, dio, relay
    };

    QMap<QString, DeviceDictType> s_mapStringToDeviceDictType =
    {
        { "motor",  DeviceDictType::motor   },
        { "scaler", DeviceDictType::scaler  },
        { "dio",    DeviceDictType::dio     },
        { "relay",  DeviceDictType::relay   }
    };

    QMap<DeviceDictType, QString> s_mapDeviceDictTypeToString =
    {
        { DeviceDictType::motor, "motor"   },
        { DeviceDictType::scaler,"scaler"  },
        { DeviceDictType::dio,   "dio"     },
        { DeviceDictType::relay, "relay"   }
    };

    /**********************  Device Motor Command Dictionary   ************************/
    enum class MotorCommandDictType
    {
        stop, position, move, motion, enabled, enable, disable, limits, home, getpos, setpos, findlim, limitshome, getpositions
    };
    QMap<QString, MotorCommandDictType> s_mapStringToMotorCommandDictType =
    {
        { "stop",       MotorCommandDictType::stop        },
        { "position",   MotorCommandDictType::position    },
        { "move",       MotorCommandDictType::move        },
        { "motion",     MotorCommandDictType::motion      },
        { "enabled",    MotorCommandDictType::enabled     },
        { "enable",     MotorCommandDictType::enable      },
        { "disable",    MotorCommandDictType::disable     },
        { "limits",     MotorCommandDictType::limits      },
        { "home",       MotorCommandDictType::home        },
        { "getpos",     MotorCommandDictType::getpos      },
        { "setpos",     MotorCommandDictType::setpos      },
        { "findlim",    MotorCommandDictType::findlim     },
        { "limitshome", MotorCommandDictType::limitshome  },
        { "getpositions",  MotorCommandDictType::getpositions}  // read multiple positions: "motor getpositions 1 2 3 4", reply: "OK:12.5 17.1 4.0 77.4"
    };

    QMap<MotorCommandDictType, QString> s_mapMotorCommandDictTypeToString =
    {
        { MotorCommandDictType::stop,       "stop"      },
        { MotorCommandDictType::position,   "position"  },
        { MotorCommandDictType::move,       "move"      },
        { MotorCommandDictType::motion,     "motion"    },
        { MotorCommandDictType::enabled,    "enabled"   },
        { MotorCommandDictType::enable,     "enable"    },
        { MotorCommandDictType::disable,    "disable"   },
        { MotorCommandDictType::limits,     "limits"    },
        { MotorCommandDictType::home,       "home"      },
        { MotorCommandDictType::getpos,     "getpos"    },
        { MotorCommandDictType::setpos,     "setpos"    },
        { MotorCommandDictType::findlim,    "findlim"   },
        { MotorCommandDictType::limitshome, "limitshome"},
        { MotorCommandDictType::getpositions,"getpositions" }
    };

    const QString ProcessMotorCommand(QStringList *pCommandAndPars);
    const QString ProcessMotorStop(QStringList *pCommandAndPars);
    const QString ProcessMotorPosition(QStringList *pCommandAndPars);
    const QString ProcessMotorMove(QStringList *pCommandAndPars);
    const QString ProcessMotorIsMoving(QStringList *pCommandAndPars);
    const QString ProcessMotorEnabled(QStringList *pCommandAndPars);
    const QString ProcessMotorEnable(QStringList *pCommandAndPars);
    const QString ProcessMotorDisable(QStringList *pCommandAndPars);
    const QString ProcessMotorLimits(QStringList *pCommandAndPars);

    // Error strings
    static const QString ERR_UNDEFINED_DEVICE;
    static const QString ERR_COMMAND_UNSUPPORTED;
    static const QString ERR_COMMAND_NOT_FOUND;
    static const QString ERR_CHANNEL_NOT_FOUND;
    static const QString ERR_PARAMETER_INVALID;
    static const QString ERR_MOTOR_DISABLED;

};

#endif // COMMANDPARSER_H
