#ifndef VIPERMOTORVIEW_H
#define VIPERMOTORVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QThread>
#include <QSignalMapper>
#include <pthread.h>
#include "MotorAxis.h"
#include "GuiMotorControl.h"
#include "GuiMotorConfig.h"
#include "CommandParser.h"
#include "CommThread.h"
#include "CommLog.h"
#include "MotorManagement.h"


class GuiMotorView : public QWidget
{
    Q_OBJECT
public:
    explicit GuiMotorView(QWidget * parent, MotorAxis * p_Axis[]);

    MotorAxis * m_Axis[NAXIS];
    QFrame * Frame;
    QPushButton * mcnum[NAXIS];
    QLabel * mcposition[NAXIS];
    QLabel * mcneglim[NAXIS];
    QLabel * mcstatus[NAXIS];
    QLabel * mcposlim[NAXIS];
    QLabel * mclabel[NAXIS];
    GuiMotorControl * mControl;
    GuiMotorConfig * mConfig;

    MotorManagement * m_MotorManager;

    CommandParser * m_Parser;
    CommThread * m_CommThread;
    CommLog * m_CommLog;

    int currentaxis;

    QTimer * timer_update;
    QTimer * timer_threads;

    void ShowControl(); // Show Motor Control Dialog
    void ShowConfig();  // Show Motor Configuration Dialog
    void ShowLog();     // Show Communications Log

signals:

public slots:
    void Refresh(void);     // Refresh Motor Status Displays
    void OnSelect(int ID);
    void onPostIncoming(QString msg);
    void onPostOutgoing(QString msg);
    void onPostError(QString msg);

};

#endif // VIPERMOTORVIEW_H
