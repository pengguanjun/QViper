#include "GuiMotorControl.h"
#include "ui_GuiMotorControl.h"

GuiMotorControl::GuiMotorControl(QWidget *parent, MotorAxis * p_Axis[], int * selectedaxis) :
    QMainWindow(parent),
    ui(new Ui::GuiMotorControl)
{
    int i;
    ui->setupUi(this);

    this->setWindowTitle(QString("MotorControl"));

    AxisComboBox    = ui->AxisComboBox;
    PositionEdit    = ui->PositionEdit;
    DestinationEdit = ui->DestinationEdit;
    IncrementEdit   = ui->IncrementEdit;
    GoButton        = ui->GoButton;
    SetButton       = ui->SetButton;
    StopButton      = ui->StopButton;
    HomeButton      = ui->HomeButton;
    NegIncButton    = ui->NegIncButton;
    PosIncButton    = ui->PosIncButton;
    NegLimStatus    = ui->StatusNegLim;
    PosLimStatus    = ui->StatusPosLim;
    StatusMsg       = ui->StatusMsg;

    currentaxis = selectedaxis;
    for (i=0;i<NAXIS;i++) {
        m_Axis[i] = p_Axis[i];
        if (m_Axis[i]->defined) {
            AxisComboBox->addItem(QString::number(i));
        }
    }

}

GuiMotorControl::~GuiMotorControl()
{
    delete ui;
}

void GuiMotorControl::Update()
{
    char szMsg[64];
    int index;

    index = *currentaxis;

    if (m_Axis[index]->parms->poslimit) {
        PosLimStatus->setStyleSheet("QLabel {background-color:red}");
    } else {
        PosLimStatus->setStyleSheet("QLabel {background-color:white}");
    }

    if (m_Axis[index]->parms->neglimit) {
        NegLimStatus->setStyleSheet("QLabel {background-color:red}");
    } else {
        NegLimStatus->setStyleSheet("QLabel {background-color:white}");
    }

    if (m_Axis[index]->parms->moving) {
        StatusMsg->setStyleSheet("QLabel {background-color:green}");
        StatusMsg->setText(QString("MOVING"));
    } else if (!m_Axis[index]->parms->axis_enabled) {
        StatusMsg->setStyleSheet("QLabel {background-color:grey}");
        StatusMsg->setText(QString("DISABLED"));
    } else if (m_Axis[index]->parms->fault) {
        StatusMsg->setStyleSheet("QLabel {background-color:yellow}");
        StatusMsg->setText(QString("FAULT"));
    } else {
        StatusMsg->setStyleSheet("QLabel {background-color:white}");
        StatusMsg->setText(QString("NORMAL"));
    }

    sprintf(szMsg,"%+.4f",m_Axis[index]->position);
    PositionEdit->setText(QString(szMsg));

    if (!DestinationEdit->hasFocus()) {
        sprintf(szMsg,"%+.4f",m_Axis[index]->target);
        DestinationEdit->setText(QString(szMsg));
    }

    if (!IncrementEdit->hasFocus()) {
        sprintf(szMsg,"%+.4f",m_Axis[index]->increment);
        IncrementEdit->setText(QString(szMsg));
    }

    return;
}

void GuiMotorControl::ShowAxis(int ID)

{
    int index, i;
    if ((ID < 0) || (ID >= NAXIS)) return;
    if (!m_Axis[ID]->defined) return;
    index=0;
    for (i=0;i<NAXIS;i++) {
        if (m_Axis[i]->defined) {
            if (ID == i) break;
            index++;
        }
    }

    *currentaxis = ID;
    this->AxisComboBox->setCurrentIndex(index);
    this->Update();
    this->show(); // If it's not shown already
}

void GuiMotorControl::on_action_Close_triggered()
{
    this->hide();
}

void GuiMotorControl::on_SetButton_clicked()
{
    int index;
    index = *currentaxis;

    if (m_Axis[index]->parms->moving) return;
    m_Axis[index]->target = DestinationEdit->text().toDouble();
    //m_Axis[index]->SetPosition(target);
    m_Axis[index]->newposition = true;
}

void GuiMotorControl::on_StopButton_clicked()
{
    int index;
    index = *currentaxis;

    m_Axis[index]->request_stop=true;
}

void GuiMotorControl::on_GoButton_clicked()
{
    int index;
    index = *currentaxis;

    if (m_Axis[index]->parms->moving) return;
    m_Axis[index]->target = DestinationEdit->text().toDouble();
    m_Axis[index]->newtarget=true; // Tell state machine to move to target
}

void GuiMotorControl::on_NegIncButton_clicked()
{
    int index;
    double position, increment;

    index = *currentaxis;
    if (m_Axis[index]->parms->moving) return;
    increment = IncrementEdit->text().toDouble();
    position = m_Axis[index]->position;
    m_Axis[index]->target = position - increment;
    m_Axis[index]->newtarget=true; // Tell state machine to move to target
}

void GuiMotorControl::on_PosIncButton_clicked()
{
    int index;
    double position, increment;

    index = *currentaxis;
    if (m_Axis[index]->parms->moving) return;
    increment = IncrementEdit->text().toDouble();
    position = m_Axis[index]->position;
    m_Axis[index]->target = position + increment;
    m_Axis[index]->newtarget=true; // Tell state machine to move to target
}

void GuiMotorControl::on_AxisComboBox_activated(int index)
{
    ShowAxis(index);
}
