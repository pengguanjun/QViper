#ifndef GUIMOTORCONTROL_H
#define GUIMOTORCONTROL_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "MotorAxis.h"

namespace Ui {
class GuiMotorControl;
}

class GuiMotorControl : public QMainWindow
{
    Q_OBJECT

public:
    explicit GuiMotorControl(QWidget *parent, MotorAxis * p_Axis[], int * selectedaxis);
    ~GuiMotorControl();

    QComboBox * AxisComboBox;
    QLineEdit * PositionEdit;
    QLineEdit * DestinationEdit;
    QLineEdit * IncrementEdit;
    QPushButton * GoButton;
    QPushButton * SetButton;
    QPushButton * StopButton;
    QPushButton * HomeButton;
    QPushButton * NegIncButton;
    QPushButton * PosIncButton;
    QLabel * NegLimStatus;
    QLabel * PosLimStatus;
    QLabel * StatusMsg;

    volatile int * currentaxis;
    volatile MotorAxis * m_Axis[NAXIS];

    void ShowAxis(int ID);
    void Update();

private slots:
    void on_action_Close_triggered();

    void on_SetButton_clicked();

    void on_StopButton_clicked();

    void on_GoButton_clicked();

    void on_NegIncButton_clicked();

    void on_PosIncButton_clicked();

    void on_AxisComboBox_activated(int index);

private:
    Ui::GuiMotorControl *ui;
};

#endif // GUIMOTORCONTROL_H
