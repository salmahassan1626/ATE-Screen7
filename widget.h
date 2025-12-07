#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
#include <QSqlDatabase>
#include <QSerialPort>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    // Your existing slots / handlers (kept names & case)
    void onNewTestButtonClicked();
    void onReportButtonClicked();
    void onGenerateButtonClicked();
    void handleBarcodeScanned();
    void handleAnyLineEditReturnPressed();
    void onNextButtonClicked();
    void openCalibriteProfiler();


    // New: Run touch test button on index 4
    void on_Runtouchtest_clicked();

    // Arduino data
    void readArduinoData();
    void onPageChanged(int index);

private:
    Ui::Widget *ui;

    // DB
    QSqlDatabase db;
    bool openDatabase();
    void createTable();
    bool updateRecordColumn(const QString &column, const QString &value);

    // Arduino & autofill
    QSerialPort *serial;
    QByteArray serialBuffer;
    int readingCount;
    double sumV;
    double sumA;
    void requestArduinoReadings();

    // App state
    int currentRecordId;
    QString currentBarcode;
    QString runFilePath;

    // Touch process
    QProcess *touchProcess;
    QDateTime startTime_ITS;
    QProcess *itsProcess = nullptr;      // Tracks ITS Studio process

    // Helpers for touch-log parsing / saving
    bool readLatestLogFile(const QString &logDirPath, QString &formattedOut, QString &resultOut);
    void saveTextToFile(const QString &text);

    // UI helpers
    void updateVisibility();
   // bool readLatestLogFile(const QString &logDirPath, QString &formattedOut, QString &resultOut);
     QProcess *itsStudioProcess;
};

#endif // WIDGET_H
