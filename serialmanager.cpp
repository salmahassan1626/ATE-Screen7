#include "serialmanager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent), serial(new QSerialPort(this))
{
    connect(serial, &QSerialPort::readyRead, this, &SerialManager::handleReadyRead);
}

void SerialManager::openSerialPort(const QString &portName)
{
    if (serial->isOpen())
        serial->close();

    serial->setPortName(portName);
    serial->setBaudRate(QSerialPort::Baud9600);  // Adjust for your device
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open port" << portName << ":" << serial->errorString();
    } else {
        qDebug() << "Connected to" << portName;
    }
}

void SerialManager::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
}

void SerialManager::handleReadyRead()
{
    QByteArray data = serial->readAll();
    emit dataReceived(QString::fromUtf8(data));
}
