#include "CommThread.h"
#include <QDebug>

CommThread::CommThread(CommandParser * p_Parser)
{

    m_Parser = p_Parser;
    m_SerialPortName = "/dev/ttyViper";
    m_BaudRate = QSerialPort::Baud9600;
    m_DataBits = QSerialPort::Data8;
    m_Parity   = QSerialPort::NoParity;
    m_StopBits = QSerialPort::OneStop;
    m_FlowControl = QSerialPort::NoFlowControl;

    //m_SerialPort.setReadBufferSize(20);

    m_IncomingSerial = "";
    m_OutgoingMsg = "";
    m_InfoMsg = "GENERIC ERROR";

    m_TCPServerPort = 10000;
}

CommThread::~CommThread()
{
    // Close serialport
    m_SerialPort->close();
    delete(m_SerialPort);
    // Should also gracefully close TCP ports as well
}

void CommThread::run()
{
    qDebug() << "Starting Comm Thread";
    m_Server = new QTcpServer();
    m_Server->listen(QHostAddress::Any,m_TCPServerPort);
    connect(m_Server, SIGNAL(newConnection()), this, SLOT(onNewTCPConnection()));

    m_SerialPort = new QSerialPort();
    m_SerialPort->setPortName(m_SerialPortName);
    m_SerialPort->setBaudRate(m_BaudRate);
    m_SerialPort->setDataBits(m_DataBits);
    m_SerialPort->setParity(m_Parity);
    m_SerialPort->setStopBits(m_StopBits);
    m_SerialPort->setFlowControl(m_FlowControl);

    if (!m_SerialPort->open(QIODevice::ReadWrite)) {
        m_InfoMsg = "Cannot open serial port: " + m_SerialPortName + "\n";
        emit onPostError(m_InfoMsg);
    } else {
        m_InfoMsg = "Opened serial port " + m_SerialPortName + "\n";
        emit onPostError(m_InfoMsg);
    }
    connect(m_SerialPort,SIGNAL(readyRead()), this, SLOT(onReadSerial()));\

    qDebug() << "Starting Comm Thread Message Loop";
    exec(); // Start message loop
}

void CommThread::onNewTCPConnection()
{
    QTcpSocket *clientSocket = m_Server->nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadTCP()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnectTCP()));

    m_Sockets.push_back(clientSocket);
    m_InfoMsg = "Connected " + clientSocket->peerName() + ":" + QString::number(clientSocket->peerPort());
    emit onPostError(m_InfoMsg);
}

void CommThread::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
        m_Sockets.removeOne(sender);
        m_InfoMsg = "Socket Closed";
        emit onPostError(m_InfoMsg);
    }
}

void CommThread::onDisconnectTCP()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    m_Sockets.removeOne(sender);
    sender->deleteLater();
    m_InfoMsg = "Socket Disconnected";
    emit onPostError(m_InfoMsg);
}

void CommThread::onReadSerial()
{
    QByteArray input;
    QByteArray output;
    qint64 byteswritten;

    input = m_SerialPort->readAll();
    //qDebug() << "onReadSerial()" << input.length() << " bytes ";

    m_IncomingSerial.append(input);
    //qDebug() << "     m_Incoming:" << m_IncomingSerial;
    if (m_IncomingSerial.contains('\n')) {
        QStringList list = m_IncomingSerial.split(QRegExp("\r\n"));
        if (list[0].length()==0) {
            m_IncomingSerial = "";
        } else {
            m_IncomingSerial = list[0];
            emit onPostIncoming(m_IncomingSerial);
            //qDebug() << "     *m_Incoming:" << m_IncomingSerial;
            m_OutgoingMsg = m_Parser->ProcessMessage(m_IncomingSerial);
            list.removeAt(0);
            //qDebug() << "     *m_Outgoing:" << m_OutgoingMsg;
            emit onPostOutgoing(m_OutgoingMsg);
            m_OutgoingMsg.append("@");
            m_OutgoingMsg.append(m_IncomingSerial);
            m_OutgoingMsg.append("\r\n");
            output = m_OutgoingMsg.toUtf8();
            byteswritten = m_SerialPort->write(output);
            qDebug() << "Wrote " << byteswritten << " to serial port";
            if (list.length() > 0) {
                m_IncomingSerial = list.join("\r\n");
            } else {
                m_IncomingSerial = "";
            }
        }
    }
}

void CommThread::onReadTCP()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray input = sender->readAll();
    m_IncomingTCP.append(input);

    if (m_IncomingTCP.contains('\n')) {
        QStringList list = m_IncomingTCP.split(QRegExp("\r\n"));
        if (list[0].length()==0) {
            m_IncomingTCP = "";
        } else {
            m_IncomingTCP = list[0];
            emit onPostIncoming(m_IncomingTCP);
            //qDebug() << "     *m_Incoming:" << m_IncomingSerial;
            m_OutgoingMsg = m_Parser->ProcessMessage(m_IncomingTCP);
            list.removeAt(0);
            //qDebug() << "     *m_Outgoing:" << m_OutgoingMsg;
            emit onPostOutgoing(m_OutgoingMsg);
            m_OutgoingMsg += "\r\n"; // Append terminator
            QByteArray outgoing = m_OutgoingMsg.toUtf8();
            sender->write(outgoing);
            if (list.length() > 0) {
                m_IncomingTCP = list.join("\r\n");
            } else {
                m_IncomingTCP = "";
            }
        }
    }
}
