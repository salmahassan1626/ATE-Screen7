#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    void openSerialPort(const QString &portName);
    void closeSerialPort();

signals:
    void dataReceived(const QString &data);

private slots:
    void handleReadyRead();

private:
    QSerialPort *serial;
};

#endif // SERIALMANAGER_H
