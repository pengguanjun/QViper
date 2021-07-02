#include <QGridLayout>
#include <QSignalMapper>
#include <QSize>
#include <QTimer>
#include <QMessageBox>

#include "GuiMotorView.h"
#include "AppConfig.h"
#include "MotorAxis.h"
#include "CommThread.h"
//#include "RemoteServer.h"

// This is the landing point for the entire GUI

GuiMotorView::GuiMotorView(QWidget * parent, MotorAxis * p_Axis[]) : QWidget(parent)
{
    int i, row;

    this->Frame = new QFrame(parent);
    QGridLayout * mainLayout = new QGridLayout();

    row = 0;
    currentaxis=-1;
    for (i=0;i<NAXIS; i++) {
        m_Axis[i] = p_Axis[i]; // Get a complete reference to the argument parameter list

        if (m_Axis[i]->defined) {
            if (currentaxis == -1) currentaxis=i;
            this->mcnum[i]      = new QPushButton();
            this->mclabel[i]    = new QLabel();
            this->mcposition[i] = new QLabel();
            this->mcneglim[i]   = new QLabel();
            this->mcposlim[i]   = new QLabel();
            this->mcstatus[i]   = new QLabel();

            mainLayout->addWidget(this->mcnum[i],row,0);
            mainLayout->addWidget(this->mclabel[i],row,1);
            mainLayout->addWidget(this->mcneglim[i],row,2);
            mainLayout->addWidget(this->mcstatus[i],row,3);
            mainLayout->addWidget(this->mcposlim[i],row,4);
            mainLayout->addWidget(this->mcposition[i],row,5);

            this->mcnum[i]->setText(QString::number(i));
            this->mcnum[i]->setFixedWidth(40);
            this->mcnum[i]->setSizePolicy(QSizePolicy::Fixed,
                                          QSizePolicy::Fixed);
            connect(this->mcnum[i], &QPushButton::clicked, [=]() { this->OnSelect(i); });

            this->mclabel[i]->setText(QString(m_Axis[i]->parms->label));
            this->mclabel[i]->setFixedWidth(150);

            this->mcneglim[i]->setText(QString("-"));
            this->mcneglim[i]->setFixedWidth(10);
            this->mcneglim[i]->setAlignment(Qt::AlignCenter);
            this->mcneglim[i]->setFrameStyle(QFrame::Raised);
            this->mcneglim[i]->setSizePolicy(QSizePolicy::Fixed,
                                             QSizePolicy::Fixed);
            this->mcneglim[i]->setLineWidth(2);
            this->mcneglim[i]->setStyleSheet("QLabel {background-color:white}");

            this->mcstatus[i]->setText(QString("NORMAL"));
            this->mcstatus[i]->setFixedWidth(100);
            this->mcstatus[i]->setAlignment(Qt::AlignCenter);
            this->mcstatus[i]->setFrameStyle(QFrame::Sunken);
            this->mcstatus[i]->setLineWidth(2);
            this->mcstatus[i]->setStyleSheet("background-color:white");

            this->mcposlim[i]->setText(QString("+"));
            this->mcposlim[i]->setFixedWidth(10);
            this->mcposlim[i]->setAlignment(Qt::AlignCenter);
            this->mcposlim[i]->setFrameStyle(QFrame::Raised);
            this->mcposlim[i]->setSizePolicy(QSizePolicy::Fixed,
                                             QSizePolicy::Fixed);
            this->mcposlim[i]->setLineWidth(2);
            this->mcposlim[i]->setStyleSheet("QLabel {background-color:white}");

            this->mcposition[i]->setText(QString("   +0.000"));
            this->mcposition[i]->setFixedWidth(120);
            this->mcposition[i]->setAlignment(Qt::AlignCenter);
            this->mcposition[i]->setFrameStyle(QFrame::Sunken);
            this->mcposition[i]->setLineWidth(2);
            this->mcposition[i]->setStyleSheet("background-color:white");

            // Connect signals with slots
            row++;
        }
    }
    setLayout(mainLayout);
    timer_update = new QTimer();

    // Set up timers for updates
    connect(timer_update,SIGNAL(timeout()), this, SLOT(Refresh()));
    timer_update->start(500); // Update every second (though could do it faster)+-

    mControl = new GuiMotorControl(this,p_Axis,&currentaxis);
    mControl->ShowAxis(currentaxis);
    mControl->hide();

    mConfig = new GuiMotorConfig(this,p_Axis,&currentaxis);
    mConfig->ShowAxis(currentaxis);
    mConfig->hide();

    m_CommLog = new CommLog(this);
    //m_CommLog->hide();

    m_MotorManager = new MotorManagement(p_Axis);

    m_MotorManager->start();

    m_Parser = new CommandParser(p_Axis);
    m_CommThread = new CommThread(m_Parser);
    connect(m_CommThread, &CommThread::onPostError, this, &GuiMotorView::onPostError);
    connect(m_CommThread, &CommThread::onPostIncoming, this, &GuiMotorView::onPostIncoming);
    connect(m_CommThread, &CommThread::onPostOutgoing, this, &GuiMotorView::onPostOutgoing);
    connect(m_CommThread, &CommThread::finished, m_CommThread, &QObject::deleteLater);

    m_CommThread->start();
}

void GuiMotorView::Refresh()
{
    char szMsg[1024];
    int i;

    // Should be a more efficient way of doing this, but...
    for (i=0;i<NAXIS;i++) {
        if (m_Axis[i]->defined) {
            // Update position
            sprintf(szMsg,"%+8.3f", m_Axis[i]->position);
            this->mcposition[i]->setText(QString(szMsg));

            // Update general status
            if (m_Axis[i]->parms->moving) {
                this->mcstatus[i]->setStyleSheet("QLabel {background-color:green}");
                this->mcstatus[i]->setText(QString("MOVING"));
            } else if (m_Axis[i]->parms->fault) {
                this->mcstatus[i]->setStyleSheet("QLabel {background-color:yellow}");
                this->mcstatus[i]->setText(QString("FAULT"));
            } else if (!m_Axis[i]->parms->axis_enabled) {
                this->mcstatus[i]->setStyleSheet("QLabel {background-color:grey}");
                this->mcstatus[i]->setText(QString("DISABLED"));
            }
            else {
                this->mcstatus[i]->setStyleSheet("QLabel {background-color:white}");
                this->mcstatus[i]->setText(QString("NORMAL"));
            }

            // Update limits
            if (m_Axis[i]->parms->poslimit) {
                this->mcposlim[i]->setStyleSheet("QLabel {background-color:red}");
            } else {
                this->mcposlim[i]->setStyleSheet("QLabel {background-color:white}");
            }
            if (m_Axis[i]->parms->neglimit) {
                this->mcneglim[i]->setStyleSheet("QLabel {background-color:red}");
            } else {
                this->mcneglim[i]->setStyleSheet("QLabel {background-color:white}");
            }


        }
    }

    // Update control widget
    mControl->Update();
}

void GuiMotorView::OnSelect(int ID)
{
    // qDebug() << "OnSelect:" << ID;
    mControl->ShowAxis(ID);
    mConfig->ShowAxis(ID);
    mControl->show();
}

void GuiMotorView::ShowControl()
{
    mControl->show();
}

void GuiMotorView::ShowConfig()
{
    mConfig->show();
}

void GuiMotorView::ShowLog()
{
    m_CommLog->show();
}

void GuiMotorView::onPostError(QString msg)
{
    m_CommLog->PostError(msg);
}

void GuiMotorView::onPostIncoming(QString msg)
{
    m_CommLog->PostIncoming(msg);
}

void GuiMotorView::onPostOutgoing(QString msg)
{
    m_CommLog->PostOutgoing(msg);
}
