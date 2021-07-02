#include "CommLog.h"
#include "ui_CommLog.h"
#include <QDebug>

CommLog::CommLog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CommLog)
{
    ui->setupUi(this);
    Transcript = ui->Transcript;
    qDebug() << "Creating commlog dialog";
    //this->hide(); // Hide by default
}

CommLog::~CommLog()
{
    delete ui;
}

void CommLog::PostError(QString msg)
{
    Transcript->setTextColor(Qt::red);
    Transcript->append(msg);
}

void CommLog::PostIncoming(QString msg)
{
    Transcript->setTextColor(Qt::blue);
    Transcript->append(msg);
}

void CommLog::PostOutgoing(QString msg)
{
    Transcript->setTextColor(Qt::green);
    Transcript->append(msg);
}

void CommLog::PostInfo(QString msg)
{
    Transcript->setTextColor(Qt::black);
    Transcript->append(msg);
}

void CommLog::on_action_Close_triggered()
{
    this->hide();
}

void CommLog::on_actionC_lear_triggered()
{
    Transcript->clear();
}
