#ifndef GUIMOTORCONFIG_H
#define GUIMOTORCONFIG_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "MotorAxis.h"

namespace Ui {
class GuiMotorConfig;
}

class GuiMotorConfig : public QMainWindow
{
    Q_OBJECT

public:
    explicit GuiMotorConfig(QWidget *parent, MotorAxis * p_Axis[], int * selectedaxis);
    ~GuiMotorConfig();

    volatile int * currentaxis;
    volatile MotorAxis * m_Axis[NAXIS];

    QLineEdit * DescriptionEdit;
    QLineEdit * DResEdit;
    QLineEdit * EResEdit;
    QLineEdit * DeadbandEdit;
    QLineEdit * DScaleEdit;
    QLineEdit * AScaleEdit;
    QLineEdit * BScaleEdit;
    QLineEdit * VScaleEdit;
    QComboBox * AxisEnableComboBox;
    QComboBox * UseEncoderComboBox;
    QComboBox * LimitsComboBox;
    QComboBox * PosMaintenanceComboBox;
    QPushButton * ApplyButton;
    QPushButton * SaveButton;

    void ShowAxis(int ID);

private slots:
    void on_action_Close_2_triggered();

    void on_ApplyButton_clicked();

    void on_AxisEnableComboBox_activated(int index);

private:
    Ui::GuiMotorConfig *ui;
};

#endif // GUIMOTORCONFIG_H
