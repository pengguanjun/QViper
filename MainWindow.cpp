#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QProcessEnvironment>
#include "AppConfig.h"

#include <unistd.h>
#include <time.h>

#define MODULE_CONFIG_FILE "module.cfg"
#define MOTOR_CONFIG_FILE  "motor.cfg"
#define MOTOR_CACHE_FILE   "motpos.dat"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    int i;
    ui->setupUi(this);

    mView    = NULL;

    this->setWindowTitle(QString("QVIPER"));

    // Initialize module parameters
    for(i=0;i<NMODS;i++) {
        ModParms[i] = new ModuleParameters();
    }

    // Initialize motor parameters
    for (i=0;i<NAXIS;i++) {
        MotParms[i] = new MotorParameters();
    }


    LoadConfig();
    SetupVME();
    SetupModules();
    SetupMotors();
    RestoreMotorPositions();
    BuildGUI();

    // Setup timers
    //timer_update = new QTimer();
    //connect(timer_update,SIGNAL(timeout()), this, SLOT(onUpdate()));
    //timer_update->start(200); // Update every 200 ms
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupVME()
{
    int retn;

    // To make life simpler for this application
    // we'll hardcode in the creation of a VMEInterface
    // and hardcode in the creation of a MAXv module
    p_VMEInterface_XXUSB_A16 = new VMEInterface_XXUSB(VMEInterface::A16);
    p_VMEInterface_XXUSB_A32 = new VMEInterface_XXUSB(VMEInterface::A32);
    retn = p_VMEInterface_XXUSB_A16->Init();
    if (!retn) {
        qDebug() << "Could not init XXUSB A16";
    }

    retn = p_VMEInterface_XXUSB_A32->Init();
    if (!retn) {
        qDebug() << "Could not init XXUSB A32";
    }

    // Just assign the interface pointer to the correct interface for the MAXv
    p_VMEInterface = (VMEInterface *) p_VMEInterface_XXUSB_A16;

}

void MainWindow::LoadConfig()
{
    QString cfgfile;

    QString homedir = QProcessEnvironment::systemEnvironment().value("HOME","/usr/local/QViper");

    cfgfile = homedir + "/" + MODULE_CONFIG_FILE;

    nmods = Mod_ConfigRead(cfgfile.toStdString().c_str(), &ModParms[0]);

    cfgfile = homedir + "/" + MOTOR_CONFIG_FILE;

    nmots = MC_ConfigRead(cfgfile.toStdString().c_str(), &MotParms[0]);

    return;

}

void MainWindow::SetupModules()
{
    int i;
    bool success;
    //QString * pModuleNo, * pModel, * pAddress, * pAS;
    QString ModuleNo, Model, Address, AS;

    for (i=0;i<NMODS;i++) {
        if (ModParms[i]->defined) {
            if (!strcmp(ModParms[i]->m_Class,"motor")) {
                ModuleNo = QString::number(nmods);
                Model    = QString(ModParms[i]->m_Model);
                Address  = QString("0x") + QString("%1").arg(ModParms[i]->m_BaseAddress,0,16);
                AS = QString("A16");
                p_MAXv[i] = new OMS_MAXv();
                success = p_MAXv[i]->Init(ModuleNo,Model,Address,AS);
                if (success) {
                    p_MAXv[i]->SetVMEInterface(p_VMEInterface_XXUSB_A16);
                    p_MAXv[i]->InitBoard();
                }
            }
        }
    }
}

int MainWindow::SetupMotors()
{
    int i,n_axes;

    n_axes = 0;
    for(i=0;i<NAXIS;i++) {
        Axis[i] = new MotorAxis();
        Axis[i]->parms = MotParms[i];
        if (MotParms[i]->defined) {
            Axis[i]->defined=true;
            int motmod = MotParms[i]->motmod;
            Axis[i]->pModule = p_MAXv[motmod];

            Axis[i]->Configure();
            n_axes++;
        }
    }

    return n_axes;
}

int MainWindow::RestoreMotorPositions()
{
    QString cfgfile;
    double position[NAXIS];
    int i, nread, nwritten;

    for (i=0;i<NAXIS;i++) { position[i] = 0; }

    QString homedir = QProcessEnvironment::systemEnvironment().value("HOME","/usr/local/QViper");
    cfgfile = homedir + "/" + MOTOR_CACHE_FILE;

    nwritten = 0;
    if ((nread = MC_PositionRead(cfgfile.toStdString().c_str(),&position[0])) <=0) return nwritten;

    for (i=0;i<NAXIS;i++) {
        if (Axis[i]->defined) {
            Axis[i]->SetPosition(position[i]);
            nwritten++;
        }
    }
    return nwritten; // Should match what we read
}

void MainWindow::SaveMotorPositions()
{
    QString homedir = QProcessEnvironment::systemEnvironment().value("HOME","/usr/local/QViper");
    QString cfgfile = homedir + "/" + MOTOR_CACHE_FILE;

    MC_PositionCache(cfgfile.toStdString().c_str(),Axis);
}

void MainWindow::onUpdate(void) {

}


void MainWindow::BuildGUI()
{
    int i;
    QScrollArea * sa = ui->scrollArea;
    mView = new GuiMotorView(sa, Axis);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setWidget(mView);

    currentaxis=0;
    for(i=0;i<NAXIS;i++) {
        if (Axis[i]->defined) {
            currentaxis = i;
            break;
        }
    }
}

void MainWindow::on_actionE_xit_triggered()
{
    SaveMotorPositions(); // Save motor positions on exit
    QCoreApplication::quit();
}

void MainWindow::on_action_Motor_Control_triggered()
{
    mView->ShowControl();
}

void MainWindow::on_actionMotor_Config_triggered()
{
    mView->ShowConfig();
}

void MainWindow::on_actionComms_Log_triggered()
{
    mView->ShowLog();
}
