// widget.cpp
#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QApplication>
#include <QIntValidator>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QtMath>
#include <QProcess>
#include <QDate>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , serial(nullptr)
    , readingCount(0)
    , sumV(0.0)
    , sumA(0.0)
    , currentRecordId(-1)
    , touchProcess(nullptr)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    // Validators
    QIntValidator *validator = new QIntValidator(0, 499, this);
    ui->voltLineEdit->setValidator(validator);
    ui->currentLineEdit->setValidator(validator);
    ui->powerLineEdit->setValidator(new QIntValidator(0, 10000, this));

    ui->XritelineEdit1->setValidator(new QIntValidator(0, 10000, this));
    ui->XritelineEdit2->setValidator(new QIntValidator(0, 10000, this));
    ui->XritelineEdit3->setValidator(new QIntValidator(0, 10000, this));


    // Connections
    connect(ui->barcodeLineEdit, &QLineEdit::returnPressed, this, &Widget::handleBarcodeScanned);
    connect(ui->voltLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->currentLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->powerLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);

    connect(ui->XritelineEdit1, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->XritelineEdit2, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->XritelineEdit3, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);

    connect(ui->nextButton, &QPushButton::clicked, this, &Widget::onNextButtonClicked);
    connect(ui->NewtestpushButton, &QPushButton::clicked, this, &Widget::onNewTestButtonClicked);
    connect(ui->reportpushButton, &QPushButton::clicked, this, &Widget::onReportButtonClicked);
    connect(ui->generatepushButton, &QPushButton::clicked, this, &Widget::onGenerateButtonClicked);
    connect(ui->Runtouchtest, &QPushButton::clicked, this, &Widget::on_Runtouchtest_clicked);

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &Widget::onPageChanged);

    // DB
    if (!openDatabase()) {
        QMessageBox::critical(this, "Database", "Failed to open database.");
    } else {
        createTable();
    }

    // Arduino Serial
    serial = new QSerialPort(this);
    serial->setPortName("COM7");
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Cannot open Arduino serial port";
    } else {
        connect(serial, &QSerialPort::readyRead, this, &Widget::readArduinoData);
        qDebug() << "Arduino serial port opened successfully";
    }

    serialBuffer.clear();
    readingCount = 0;
    sumV = sumA = 0.0;
    currentRecordId = -1;
    currentBarcode.clear();
    runFilePath.clear();

    updateVisibility();
}

Widget::~Widget()
{
    if (db.isOpen()) db.close();
    if (touchProcess) {
        touchProcess->deleteLater();
        touchProcess = nullptr;
    }
    delete ui;
}

// -------------------- UI handlers --------------------
void Widget::onNewTestButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->barcodeLineEdit->clear();
    ui->barcodeLineEdit->setVisible(true);
    ui->barcodeLineEdit->setFocus();
    currentBarcode.clear();
    currentRecordId = -1;
    runFilePath.clear();
    ui->ScannedBarCode_2->setText("Please scan the barcode");
    updateVisibility();
}

void Widget::onReportButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(6);
    ui->reportBarCodelineEdit->clear();
    updateVisibility();
}

void Widget::onGenerateButtonClicked()
{
    QString barcode = ui->reportBarCodelineEdit->text().trimmed();
    if (barcode.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a barcode.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT testNumber, backlight_voltage, backlight_current, power_watts, touch_test, rgb, luminance, contrast "
                  "FROM records WHERE barcode = :barcode ORDER BY testNumber ASC");
    query.bindValue(":barcode", barcode);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to fetch data: " + query.lastError().text());
        return;
    }

    if (!query.next()) {
        QMessageBox::information(this, "No Records", "No test records found for this barcode.");
        return;
    }
    query.previous();

    QFileInfo dbInfo(db.databaseName());
    QString dbDir = dbInfo.absolutePath();
    QString reportsDirPath = QDir(dbDir).filePath("Reports");
    QDir reportsDir(reportsDirPath);
    if (!reportsDir.exists()) {
        if (!reportsDir.mkpath(".")) {
            QMessageBox::warning(this, "File Error", "Unable to create Reports directory: " + reportsDirPath);
            return;
        }
    }

    QString filePath = reportsDir.filePath("report_" + barcode + ".txt");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Unable to write report file: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << "Report for Barcode: " << barcode << "\n\n";

    while (query.next()) {
        int testNum = query.value(0).toInt();
        QString volt = query.value(1).toString();
        QString curr = query.value(2).toString();
        QString watts = query.value(3).toString();
        QString touch = query.value(4).toString();
        QString rgb = query.value(5).toString();
        QString luminance = query.value(6).toString();
        QString contrast = query.value(7).toString();

        out << "Test #" << testNum << "\n";
        out << "Voltage: " << volt << "\n";
        out << "Current: " << curr << "\n";
        out << "Power (W): " << watts << "\n";
        out << "Touch Test: " << touch << "\n";
        out << "RGB: " << rgb << "\n";
        out << "Luminance: " << luminance << "\n";
        out << "Contrast: " << contrast << "\n\n";
    }

    out << "--- End of Report ---\n";
    file.close();

    QMessageBox::information(this, "Report Generated", "Report saved as " + QFileInfo(filePath).fileName());
    ui->stackedWidget->setCurrentIndex(5);
    updateVisibility();
}

// -------------------- barcode --------------------
void Widget::handleBarcodeScanned()
{
    QString barcode = ui->barcodeLineEdit->text().trimmed();
    if (barcode.isEmpty()) return;

    currentBarcode = barcode;

    int testNumber = 1;
    QSqlQuery countQuery(db);
    countQuery.prepare("SELECT MAX(testNumber) FROM records WHERE barcode = :barcode");
    countQuery.bindValue(":barcode", barcode);
    if (countQuery.exec() && countQuery.next()) {
        testNumber = countQuery.value(0).toInt() + 1;
        if (testNumber <= 0) testNumber = 1;
    }

    ui->ScannedBarCode_2->setText("Scanned: " + barcode);

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO records (barcode, testNumber) VALUES (:barcode, :testNumber)");
    insertQuery.bindValue(":barcode", barcode);
    insertQuery.bindValue(":testNumber", testNumber);
    if (!insertQuery.exec()) {
        qDebug() << "Insert failed:" << insertQuery.lastError().text();
    }

    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT id FROM records WHERE barcode = :barcode AND testNumber = :testNumber ORDER BY id DESC LIMIT 1");
    selectQuery.bindValue(":barcode", barcode);
    selectQuery.bindValue(":testNumber", testNumber);
    if (selectQuery.exec() && selectQuery.next()) {
        currentRecordId = selectQuery.value(0).toInt();
    } else {
        currentRecordId = -1;
    }

    ui->barcodeLineEdit->clear();
    ui->barcodeLineEdit->setVisible(false);

    requestArduinoReadings();
    updateVisibility();
}

// -------------------- lineedit --------------------
void Widget::handleAnyLineEditReturnPressed()
{
    QLineEdit *line = qobject_cast<QLineEdit *>(sender());
    if (!line || currentRecordId == -1) return;

    QString text = line->text().trimmed();
    if (text.isEmpty()) return;

    QString column;
    if (line == ui->voltLineEdit || line == ui->currentLineEdit || line == ui->XritelineEdit1 || line == ui->XritelineEdit2 || line == ui->XritelineEdit3) {
        bool ok;
        int val = text.toInt(&ok);
        if (!ok || val >= 500) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a number less than 500.");
            line->clear();
            return;
        }
    }

    if (line == ui->voltLineEdit) column = "backlight_voltage";
    else if (line == ui->currentLineEdit) column = "backlight_current";
    else if (line == ui->powerLineEdit) column = "power_watts";

    else if (line == ui->XritelineEdit1) column = "rgb";
    else if (line == ui->XritelineEdit2) column = "luminance";
    else if (line == ui->XritelineEdit3) column = "contrast";
    else return;



    if (!updateRecordColumn(column, text)) {
        qDebug() << "Failed to update column" << column;
    }
    line->clear();
}
//-------------------- ---------------------
void Widget::onPageChanged(int index)
{
    if (index == 4) {
        openCalibriteProfiler();
    }
}

// -------------------- next button --------------------
void Widget::onNextButtonClicked()
{
    int index = ui->stackedWidget->currentIndex();

    if (index == 1 && currentBarcode.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please scan the barcode before proceeding.");
        return;
    }

    if (index == 2) {
        QString volt = ui->voltLineEdit->text().trimmed();
        QString current = ui->currentLineEdit->text().trimmed();
        QString power = ui->powerLineEdit->text().trimmed();

        if (volt.isEmpty() || current.isEmpty() || power.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Arduino readings not available. Enter values manually.");
            return;
        }

        updateRecordColumn("backlight_voltage", volt);
        updateRecordColumn("backlight_current", current);
        updateRecordColumn("power_watts", power);

        ui->voltLineEdit->clear();
        ui->currentLineEdit->clear();
        ui->powerLineEdit->clear();
    }


    if (index == 4) {

      //  openCalibriteProfiler();

        QString rgb = ui->XritelineEdit1->text().trimmed();
        QString luminance = ui->XritelineEdit2->text().trimmed();
        QString contrast = ui->XritelineEdit3->text().trimmed();

        if (rgb.isEmpty() || luminance.isEmpty() || contrast.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Xrite reading is not available. Enter values manually.");
            return;
        }

        updateRecordColumn("rgb", rgb);
        updateRecordColumn("luminance", luminance);
        updateRecordColumn("contrast", contrast);

        ui->XritelineEdit1->clear();
        ui->XritelineEdit2->clear();
        ui->XritelineEdit3->clear();
    }

    if (index == 5) {
        QApplication::quit();
        return;
    }

    ui->stackedWidget->setCurrentIndex(index + 1);
    updateVisibility();
}

// -------------------- DB --------------------
bool Widget::openDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QCoreApplication::applicationDirPath() + "/../../mydatabase.db";
    db.setDatabaseName(path);
    bool ok = db.open();
    if (!ok) qDebug() << "Failed to open DB at" << path << ":" << db.lastError().text();
    else qDebug() << "Database opened at:" << path;
    return ok;
}

void Widget::createTable()
{
    QSqlQuery query(db);

    // ---- CREATE TABLE FIXED ----
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS records ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "barcode TEXT, "
            "testNumber INTEGER, "
            "backlight_voltage TEXT, "
            "backlight_current TEXT, "
            "power_watts TEXT, "
            "touch_test TEXT, "
            "rgb TEXT, "
            "luminance TEXT, "
            "contrast TEXT"
            ");"                       // <-- FIXED: closing parenthesis
            )) {
        qDebug() << "Create table failed:" << query.lastError().text();
    }

    // ---- CLEAN FILTER ROWS FIXED ----
    QSqlQuery del(db);
    if (!del.exec(
            "DELETE FROM records "
            "WHERE barcode = 'filter' "
            "AND backlight_voltage = 'filter' "
            "AND backlight_current = 'filter' "
            "AND power_watts = 'filter' "
            "AND touch_test = 'filter' "
            "AND rgb = 'filter' "
            "AND luminance = 'filter' "
            "AND contrast = 'filter'"
            )) {
        qDebug() << "Failed to clean filter rows:" << del.lastError().text();
    }

    // ---- CHECK AND ADD MISSING COLUMNS ----
    QSqlQuery checkQuery(db);
    checkQuery.exec("PRAGMA table_info(records)");

    bool hasPowerWatts = false;
    bool hasRGB = false;
    bool hasLuminance = false;
    bool hasContrast = false;

    while (checkQuery.next()) {
        QString colName = checkQuery.value(1).toString().toLower();

        if (colName == "power_watts")  hasPowerWatts = true;
        if (colName == "rgb")          hasRGB = true;
        if (colName == "luminance")    hasLuminance = true;
        if (colName == "contrast")     hasContrast = true;
    }

    if (!hasPowerWatts) {
        QSqlQuery alter(db);
        alter.exec("ALTER TABLE records ADD COLUMN power_watts TEXT");
    }
    if (!hasRGB) {
        QSqlQuery alter(db);
        alter.exec("ALTER TABLE records ADD COLUMN rgb TEXT");
    }
    if (!hasLuminance) {
        QSqlQuery alter(db);
        alter.exec("ALTER TABLE records ADD COLUMN luminance TEXT");
    }
    if (!hasContrast) {
        QSqlQuery alter(db);
        alter.exec("ALTER TABLE records ADD COLUMN contrast TEXT");
    }
}

    // ---- CalibriteProfiler ----


void Widget::openCalibriteProfiler()
{
    // Full path to the EXE (use double backslashes)
    QString programPath = "C:\\Program Files\\calibrite PROFILER\\calibrite PROFILER.exe";

    // Check if the file exists
    if (!QFile::exists(programPath)) {
        QMessageBox::warning(this, "File not found", programPath);
        return;
    }

    // Start the EXE directly — safest and simplest
    bool started = QProcess::startDetached(programPath);

    if (!started) {
        QMessageBox::critical(this, "Error", "Failed to start Calibrite PROFILER.");
    }
}


bool Widget::updateRecordColumn(const QString &column, const QString &value)
{
    if (currentRecordId == -1) return false;
    QSqlQuery query(db);
    QString sql = QString("UPDATE records SET %1 = :val WHERE id = :id").arg(column);
    query.prepare(sql);
    query.bindValue(":val", value);
    query.bindValue(":id", currentRecordId);
    if (!query.exec()) {
        qDebug() << "Update failed:" << query.lastError().text();
        return false;
    }
    return true;
}

void Widget::updateVisibility()
{
    int index = ui->stackedWidget->currentIndex();
    ui->nextButton->setVisible(index != 0);
    ui->ScannedBarCode_2->setVisible(index >= 1);
    if (index == 1 && currentBarcode.isEmpty())
        ui->ScannedBarCode_2->setText("Please scan the barcode");
}

// -------------------- Arduino --------------------
void Widget::requestArduinoReadings()
{
    if (serial && serial->isOpen()) {
        serialBuffer.clear();
        sumV = 0.0;
        sumA = 0.0;
        readingCount = 0;

        serial->write("R");
        qDebug() << "Requested Arduino readings...";
    } else {
        qDebug() << "Serial not open for Arduino";
    }
}

void Widget::readArduinoData()
{
    if (!serial || !serial->isOpen()) return;

    QByteArray newData = serial->readAll();
    serialBuffer.append(newData);
    qDebug() << "Received raw data from Arduino:" << newData;

    while (serialBuffer.contains('\n')) {
        int idx = serialBuffer.indexOf('\n');
        QByteArray lineData = serialBuffer.left(idx).trimmed();
        serialBuffer.remove(0, idx + 1);

        QString line = QString::fromUtf8(lineData);
        qDebug() << "Parsed line from Arduino:" << line;

        double v = 0.0, a = 0.0, p = 0.0;
        QRegularExpression rxV("V=\\s*(-?[0-9]+\\.?[0-9]*)");
        QRegularExpression rxA("A=\\s*(-?[0-9]+\\.?[0-9]*)");
        QRegularExpression rxP("W=\\s*(-?[0-9]+\\.?[0-9]*)");

        QRegularExpressionMatch mV = rxV.match(line);
        QRegularExpressionMatch mA = rxA.match(line);
        QRegularExpressionMatch mP = rxP.match(line);

        if (mV.hasMatch()) v = mV.captured(1).toDouble();
        if (mA.hasMatch()) a = mA.captured(1).toDouble();
        if (mP.hasMatch()) p = mP.captured(1).toDouble();

        double vRounded = qBound(0.0, qRound(v * 100.0) / 100.0, 499.99);
        double aRounded = qBound(0.0, qRound(a * 100.0) / 100.0, 499.99);
        double pRounded = qMax(0.0, qRound(p * 100.0) / 100.0);

        ui->voltLineEdit->setText(QString::number(vRounded, 'f', 2));
        ui->currentLineEdit->setText(QString::number(aRounded, 'f', 2));
        ui->powerLineEdit->setText(QString::number(pRounded, 'f', 2));

        updateRecordColumn("backlight_voltage", QString::number(vRounded));
        updateRecordColumn("backlight_current", QString::number(aRounded));
        updateRecordColumn("power_watts", QString::number(pRounded));

        qDebug() << "Arduino values autofilled and saved to DB: V=" << vRounded << " A=" << aRounded << " P=" << pRounded;
    }
}

// -------------------- Run touch test --------------------

void Widget::on_Runtouchtest_clicked()
{
    //Locate ITS Studio
    QString appDir = QCoreApplication::applicationDirPath();
    QDir parentDir(appDir);
    parentDir.cdUp(); parentDir.cdUp(); parentDir.cdUp();
    QString usbFolder = parentDir.filePath("TO-09-215_TP-DD0700-A21_16W-2021Y_USB");

    QString exePath = QDir(usbFolder).filePath("ITS Studio.exe");
    if (!QFile::exists(exePath)) {
        QMessageBox::warning(this, "Error", "Touch test executable not found:\n" + exePath);
        return;
    }

    //Launch ITS Studio only if not already running
    if (!itsProcess) {
        itsProcess = new QProcess(this);
        bool started = itsProcess->startDetached(exePath, {}, usbFolder);
        if (!started) {
            QMessageBox::warning(this, "Error", "Failed to launch touch test:\n" + exePath);
            itsProcess->deleteLater();
            itsProcess = nullptr;
            return;
        }
        // Record start time immediately after launching
        startTime_ITS = QDateTime::currentDateTime();
        qDebug() << "ITS Studio start time =" << startTime_ITS;
    }

    //Update UI
    ui->nextButton->setEnabled(false);
    ui->touchResultlineEdit->setText("Running touch test...");

    //Prepare log folder
    QString todayFolder = QDir(QDir(usbFolder).filePath("Log"))
                              .filePath(QDate::currentDate().toString("yyyy_MM_dd"));
    QDir logDir(todayFolder);

    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            QMessageBox::warning(this, "Error", "Cannot create log folder:\n" + todayFolder);
            return;
        }
    }

    //Setup QFileSystemWatcher
    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    watcher->addPath(todayFolder);

    // Lambda to check for new log files
    auto checkNewFile = [=]() {
        QFileInfoList files = logDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
        if (files.isEmpty()) return;

        QFileInfo latest = files.first();
        if (latest.lastModified() >= startTime_ITS) {
            QString base = latest.completeBaseName().toLower(); // e.g., "log_1234_pass"
            QStringList parts = base.split("_", Qt::SkipEmptyParts);
            if (parts.isEmpty()) return;

            QString result = parts.last(); // last part of filename
            if (result == "pass" || result == "fail") {
                ui->touchResultlineEdit->setText(result);
                updateRecordColumn("touch_test", result);
                saveTextToFile(result);
                ui->nextButton->setEnabled(true);
                watcher->deleteLater();
            }
        }
    };

    //Connect watcher signal
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, [=](const QString &) {
        checkNewFile();
    });

    //Immediate check in case file exists quickly
    QTimer::singleShot(500, this, [=]() { checkNewFile(); });
}



void Widget::saveTextToFile(const QString &text)
{
    QFile file("output.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file to save text");
        return;
    }
    QTextStream out(&file);
    out << text << "\n";
    file.close();
}
