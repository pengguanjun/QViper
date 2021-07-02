#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QGroupBox>
#include <QScrollArea>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "AppDef.h"
#include "MotorParameters.h"
#include "ModuleParameters.h"
#include "VMEInterface.h"
#include "VMEInterface_XXUSB.h"
#include "MotorAxis.h"
#include "OMS_MAXv.h"
#include "CommLog.h"
#include "GuiMotorView.h"
#include "GuiMotorControl.h"
#include "GuiMotorConfig.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QScrollArea * ScrollArea;

    GuiMotorView * mView;

    int nmots;
    int nmods;
    MotorAxis * Axis[NAXIS];
    MotorParameters * MotParms[NAXIS];
    ModuleParameters * ModParms[NMODS];
    OMS_MAXv * p_MAXv[NMODS]; // Should be motor module...

    VMEInterface * p_VMEInterface;

    QTimer * timer_update;

    int currentaxis;

    void Init();
    void LoadConfig();
    void SaveConfig();
    void SetupVME();
    void SetupModules();
    int  SetupMotors();
    int  RestoreMotorPositions();
    void SaveMotorPositions();
    void BuildGUI();

private slots:
    void onUpdate();

    void on_actionE_xit_triggered();
    void on_action_Motor_Control_triggered();
    void on_actionMotor_Config_triggered();
    void on_actionComms_Log_triggered();

private:
    Ui::MainWindow *ui;
    VMEInterface_XXUSB * p_VMEInterface_XXUSB_A16;
    VMEInterface_XXUSB * p_VMEInterface_XXUSB_A32;

};
#endif // MAINWINDOW_H
