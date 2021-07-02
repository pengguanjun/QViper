#include "GuiMotorConfig.h"
#include "ui_GuiMotorConfig.h"
#include "MotorAxis.h"

GuiMotorConfig::GuiMotorConfig(QWidget *parent, MotorAxis * p_Axis[], int * selectedaxis) :
    QMainWindow(parent),
    ui(new Ui::GuiMotorConfig)
{
    int i;
    ui->setupUi(this);

    currentaxis = selectedaxis;

    DescriptionEdit = ui->DescriptionEdit;
    DResEdit = ui->DResEdit;
    EResEdit = ui->EResEdit;
    DeadbandEdit = ui->DeadbandEdit;
    DScaleEdit = ui->DScaleEdit;
    AScaleEdit = ui->AScaleEdit;
    BScaleEdit = ui->BScaleEdit;
    VScaleEdit = ui->VScaleEdit;
    AxisEnableComboBox = ui->AxisEnableComboBox;
    UseEncoderComboBox = ui->UseEncoderComboBox;
    LimitsComboBox = ui->LimtsComboBox;
    PosMaintenanceComboBox = ui->PosMaintenanceComboBox;
    ApplyButton = ui->ApplyButton;
    SaveButton = ui->SaveButton;

    for (i=0;i<NAXIS;i++) {
        m_Axis[i] = p_Axis[i];
    }
}

GuiMotorConfig::~GuiMotorConfig()
{
    delete ui;
}

void GuiMotorConfig::ShowAxis(int ID) {
    QString msg;
    int val;

    msg = QString(m_Axis[ID]->parms->label);
    DescriptionEdit->setText(msg);

    msg = QString::number(m_Axis[ID]->parms->MotorStepsPerRevolution);
    DResEdit->setText(msg);

    msg = QString::number(m_Axis[ID]->parms->EncoderStepsPerRevolution);
    EResEdit->setText(msg);

    msg = QString::number(m_Axis[ID]->parms->deadband);
    DeadbandEdit->setText(msg);

    msg = QString::asprintf("%.4f",m_Axis[ID]->parms->dscale);
    DScaleEdit->setText(msg);

    msg = QString::asprintf("%.4f",m_Axis[ID]->parms->ascale);
    AScaleEdit->setText(msg);

    msg = QString::asprintf("%.4f",m_Axis[ID]->parms->bscale);
    BScaleEdit->setText(msg);

    msg = QString::asprintf("%.4f",m_Axis[ID]->parms->vscale);
    VScaleEdit->setText(msg);

    val = (m_Axis[ID]->parms->axis_enabled) ? 1 : 0;
    AxisEnableComboBox->setCurrentIndex(val);

    val = (m_Axis[ID]->parms->use_encoder) ? 1 : 0;
    UseEncoderComboBox->setCurrentIndex(val);

    val = (m_Axis[ID]->parms->limits_enabled) ? 1 : 0;
    LimitsComboBox->setCurrentIndex(val);

    val = (m_Axis[ID]->parms->positionmaintenance) ? 1 : 0;
    PosMaintenanceComboBox->setCurrentIndex(val);

    return;
}

void GuiMotorConfig::on_action_Close_2_triggered()
{
    this->hide();
}

void GuiMotorConfig::on_ApplyButton_clicked()
{
    int ID;
    ID = *currentaxis;

    m_Axis[ID]->writeconfig = true;
    return;
}

void GuiMotorConfig::on_AxisEnableComboBox_activated(int index)
{
    int ID;
    ID = *currentaxis;
    m_Axis[ID]->targetstate = (index) ? true : false;
    m_Axis[ID]->newaxisenable = true;
    return;
}
