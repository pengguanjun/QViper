#ifndef COMMTHREAD_H
#define COMMTHREAD_H

#include <QThread>
#include <QObject>
#include <QTime>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSerialPort>
#include "CommandParser.h"

class CommThread : public QThread
{
    Q_OBJECT
public:
    CommThread(CommandParser * p_Parser);
    ~CommThread();
    void run() override;

    // Incoming command handler
    CommandParser * m_Parser;

    // Network Interface
    QTcpServer * m_Server;
    QList<QTcpSocket *> m_Sockets;
    int m_TCPServerPort;

    // Serial Interface
    QSerialPort * m_SerialPort;
    QString m_SerialPortName;
    QSerialPort::BaudRate m_BaudRate;
    QSerialPort::DataBits m_DataBits;
    QSerialPort::Parity   m_Parity;
    QSerialPort::StopBits m_StopBits;
    QSerialPort::FlowControl m_FlowControl;

    QString m_IncomingSerial;
    QString m_IncomingTCP; // Assume one at a time
    QString m_OutgoingMsg;
    QString m_InfoMsg;

signals:
    void onPostIncoming(QString msg); // blue
    void onPostOutgoing(QString msg); // red
    void onPostError(QString msg);    // green

public slots:
    void onReadSerial();
    void onReadTCP();
    void onNewTCPConnection();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onDisconnectTCP();
};

#endif // COMMTHREAD_H
