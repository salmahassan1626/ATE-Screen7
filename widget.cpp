/*#include "widget.h"
#include "ui_widget.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QApplication>
#include <QIntValidator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

    QIntValidator *validator = new QIntValidator(0, 499, this);
    ui->voltLineEdit->setValidator(validator);
    ui->currentLineEdit->setValidator(validator);

    // Setup ComboBox items
    ui->touchComboBox->clear();
    ui->touchComboBox->addItem("Select...");
    ui->touchComboBox->addItem("pass");
    ui->touchComboBox->addItem("fail");
    ui->touchComboBox->setCurrentIndex(0);

    connect(ui->barcodeLineEdit, &QLineEdit::returnPressed, this, &Widget::handleBarcodeScanned);
    connect(ui->voltLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->currentLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->nextButton, &QPushButton::clicked, this, &Widget::onNextButtonClicked);
    connect(ui->NewtestpushButton, &QPushButton::clicked, this, &Widget::onNewTestButtonClicked);

    if (!openDatabase()) {
        QMessageBox::critical(this, "Database", "Failed to open database.");
    } else {
        createTable();
    }

    updateVisibility();
}

Widget::~Widget()
{
    if (db.isOpen()) db.close();
    delete ui;
}

void Widget::onNewTestButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->barcodeLineEdit->clear();
    ui->barcodeLineEdit->setVisible(true);
    ui->barcodeLineEdit->setFocus();
    currentBarcode.clear();
    currentRecordId = -1;
    ui->ScannedBarCode_2->setText("Please scan the barcode");
    updateVisibility();
}

void Widget::handleBarcodeScanned()
{
    QString barcode = ui->barcodeLineEdit->text().trimmed();
    if (!barcode.isEmpty()) {
        currentBarcode = barcode;
        ui->ScannedBarCode_2->setText("Scanned: " + barcode);
        saveTextToFile("Barcode", barcode);
        insertBarcode(barcode);

        ui->barcodeLineEdit->clear();
        ui->barcodeLineEdit->setVisible(false);
    }
}

void Widget::handleAnyLineEditReturnPressed()
{
    QLineEdit *line = qobject_cast<QLineEdit *>(sender());
    if (!line || currentRecordId == -1) return;

    QString text = line->text().trimmed();
    if (text.isEmpty()) return;

    QString label, column;

    if (line == ui->voltLineEdit || line == ui->currentLineEdit) {
        bool ok;
        int value = text.toInt(&ok);
        if (!ok || value >= 500) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a number less than 500.");
            line->clear();
            return;
        }
    }

    if (line == ui->voltLineEdit) {
        label = "Volt";
        column = "backlight_voltage";
    } else if (line == ui->currentLineEdit) {
        label = "Current";
        column = "backlight_current";
    }

    saveTextToFile(label, text);
    updateRecordColumn(column, text);
    line->clear();
}

void Widget::onNextButtonClicked()
{
    int index = ui->stackedWidget->currentIndex();

    if (index == 1) {
        if (currentBarcode.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please scan the barcode before proceeding.");
            return;
        }
    }
    else if (index == 2) {
        QString volt = ui->voltLineEdit->text().trimmed();
        QString current = ui->currentLineEdit->text().trimmed();

        bool voltOk, currentOk;
        int voltValue = volt.toInt(&voltOk);
        int currentValue = current.toInt(&currentOk);

        if (!voltOk || voltValue >= 500) {
            QMessageBox::warning(this, "Input Error", "Please enter a valid voltage below 500.");
            return;
        }

        if (!currentOk || currentValue >= 500) {
            QMessageBox::warning(this, "Input Error", "Please enter a valid current below 500.");
            return;
        }

        saveTextToFile("Volt", volt);
        saveTextToFile("Current", current);
        updateRecordColumn("backlight_voltage", volt);
        updateRecordColumn("backlight_current", current);

        ui->voltLineEdit->clear();
        ui->currentLineEdit->clear();
    }
    else if (index == 3) {
        QString touch = ui->touchComboBox->currentText().trimmed().toLower();

        if (touch != "pass" && touch != "fail") {
            QMessageBox::warning(this, "Input Error", "Please select either 'pass' or 'fail' for the touch test.");
            return;
        }

        saveTextToFile("Touch", touch);
        updateRecordColumn("touch_test", touch);
    }

    if (index < ui->stackedWidget->count() - 1) {
        ui->stackedWidget->setCurrentIndex(index + 1);
        updateVisibility();
    } else {
        QApplication::quit();
    }
}

bool Widget::openDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QCoreApplication::applicationDirPath() + "/../../mydatabase.db";
    db.setDatabaseName(dbPath);
    qDebug() << "Opening DB at:" << dbPath;
    return db.open();
}

void Widget::createTable()
{
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "barcode TEXT, "
               "backlight_voltage TEXT, "
               "backlight_current TEXT, "
               "touch_test TEXT)");
}

void Widget::saveTextToFile(const QString &label, const QString &text)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/../../data.txt";
    QFile file(filePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << label << ": " << text << "\n";
        file.close();
    }
}

bool Widget::insertBarcode(const QString &barcode)
{
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO records (barcode) VALUES (:barcode)");
    insertQuery.bindValue(":barcode", barcode);

    if (!insertQuery.exec()) {
        qDebug() << "Insert failed:" << insertQuery.lastError().text();
        return false;
    }

    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT id FROM records WHERE barcode = :barcode ORDER BY id DESC LIMIT 1");
    selectQuery.bindValue(":barcode", barcode);

    if (selectQuery.exec() && selectQuery.next()) {
        currentRecordId = selectQuery.value(0).toInt();
        qDebug() << "Inserted barcode with ID:" << currentRecordId;
        return true;
    } else {
        qDebug() << "Failed to retrieve inserted ID:" << selectQuery.lastError().text();
        return false;
    }
}

bool Widget::updateRecordColumn(const QString &column, const QString &value)
{
    if (currentRecordId == -1) return false;

    QSqlQuery query(db);
    QString sql = QString("UPDATE records SET %1 = :value WHERE id = :id").arg(column);
    query.prepare(sql);
    query.bindValue(":value", value);
    query.bindValue(":id", currentRecordId);
    return query.exec();
}

void Widget::updateVisibility()
{
    int index = ui->stackedWidget->currentIndex();
    ui->nextButton->setVisible(index != 0);
    ui->ScannedBarCode_2->setVisible(index >= 1);

    if (index == 1 && currentBarcode.isEmpty()) {
        ui->ScannedBarCode_2->setText("Please scan the barcode");
    }
}
*/
/*
#include "widget.h"
#include "ui_widget.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QApplication>
#include <QIntValidator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

    QIntValidator *validator = new QIntValidator(0, 499, this);
    ui->voltLineEdit->setValidator(validator);
    ui->currentLineEdit->setValidator(validator);

    ui->touchComboBox->clear();
    ui->touchComboBox->addItem("Select...");
    ui->touchComboBox->addItem("pass");
    ui->touchComboBox->addItem("fail");
    ui->touchComboBox->setCurrentIndex(0);

    connect(ui->barcodeLineEdit, &QLineEdit::returnPressed, this, &Widget::handleBarcodeScanned);
    connect(ui->voltLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->currentLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->nextButton, &QPushButton::clicked, this, &Widget::onNextButtonClicked);
    connect(ui->NewtestpushButton, &QPushButton::clicked, this, &Widget::onNewTestButtonClicked);

    if (!openDatabase()) {
        QMessageBox::critical(this, "Database", "Failed to open database.");
    } else {
        createTable();
    }

    updateVisibility();
}

Widget::~Widget()
{
    if (db.isOpen()) db.close();
    delete ui;
}

void Widget::onNewTestButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->barcodeLineEdit->clear();
    ui->barcodeLineEdit->setVisible(true);
    ui->barcodeLineEdit->setFocus();
    currentBarcode.clear();
    currentRecordId = -1;
    ui->ScannedBarCode_2->setText("Please scan the barcode");
    updateVisibility();
}

void Widget::handleBarcodeScanned()
{
    QString barcode = ui->barcodeLineEdit->text().trimmed();
    if (!barcode.isEmpty()) {
        currentBarcode = barcode;

        // Determine next testNumber
        int testNumber = 1;
        QSqlQuery query(db);
        query.prepare("SELECT MAX(testNumber) FROM records WHERE barcode = :barcode");
        query.bindValue(":barcode", barcode);
        if (query.exec() && query.next()) {
            int maxVal = query.value(0).toInt();
            testNumber = maxVal + 1;
        }

        ui->ScannedBarCode_2->setText("Scanned: " + barcode);
        saveTextToFile("Barcode", barcode);
        saveTextToFile("TestNumber", QString::number(testNumber));

        QSqlQuery insertQuery(db);
        insertQuery.prepare("INSERT INTO records (barcode, testNumber) VALUES (:barcode, :testNumber)");
        insertQuery.bindValue(":barcode", barcode);
        insertQuery.bindValue(":testNumber", testNumber);

        if (!insertQuery.exec()) {
            qDebug() << "Insert failed:" << insertQuery.lastError().text();
            return;
        }

        QSqlQuery selectQuery(db);
        selectQuery.prepare("SELECT id FROM records WHERE barcode = :barcode AND testNumber = :testNumber ORDER BY id DESC LIMIT 1");
        selectQuery.bindValue(":barcode", barcode);
        selectQuery.bindValue(":testNumber", testNumber);
        if (selectQuery.exec() && selectQuery.next()) {
            currentRecordId = selectQuery.value(0).toInt();
        } else {
            qDebug() << "Failed to retrieve inserted ID:" << selectQuery.lastError().text();
        }

        ui->barcodeLineEdit->clear();
        ui->barcodeLineEdit->setVisible(false);
    }
}

void Widget::handleAnyLineEditReturnPressed()
{
    QLineEdit *line = qobject_cast<QLineEdit *>(sender());
    if (!line || currentRecordId == -1) return;

    QString text = line->text().trimmed();
    if (text.isEmpty()) return;

    QString label, column;

    if (line == ui->voltLineEdit || line == ui->currentLineEdit) {
        bool ok;
        int value = text.toInt(&ok);
        if (!ok || value >= 500) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a number less than 500.");
            line->clear();
            return;
        }
    }

    if (line == ui->voltLineEdit) {
        label = "Volt";
        column = "backlight_voltage";
    } else if (line == ui->currentLineEdit) {
        label = "Current";
        column = "backlight_current";
    }

    saveTextToFile(label, text);
    updateRecordColumn(column, text);
    line->clear();
}

void Widget::onNextButtonClicked()
{
    int index = ui->stackedWidget->currentIndex();

    if (index == 1) {
        if (currentBarcode.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please scan the barcode before proceeding.");
            return;
        }
    }
    else if (index == 2) {
        QString volt = ui->voltLineEdit->text().trimmed();
        QString current = ui->currentLineEdit->text().trimmed();

        bool voltOk, currentOk;
        int voltValue = volt.toInt(&voltOk);
        int currentValue = current.toInt(&currentOk);

        if (!voltOk || voltValue >= 500) {
            QMessageBox::warning(this, "Input Error", "Please enter a valid voltage below 500.");
            return;
        }

        if (!currentOk || currentValue >= 500) {
            QMessageBox::warning(this, "Input Error", "Please enter a valid current below 500.");
            return;
        }

        saveTextToFile("Volt", volt);
        saveTextToFile("Current", current);
        updateRecordColumn("backlight_voltage", volt);
        updateRecordColumn("backlight_current", current);

        ui->voltLineEdit->clear();
        ui->currentLineEdit->clear();
    }
    else if (index == 3) {
        QString touch = ui->touchComboBox->currentText().trimmed().toLower();

        if (touch != "pass" && touch != "fail") {
            QMessageBox::warning(this, "Input Error", "Please select either 'pass' or 'fail' for the touch test.");
            return;
        }

        saveTextToFile("Touch", touch);
        updateRecordColumn("touch_test", touch);
    }

    if (index < ui->stackedWidget->count() - 1) {
        ui->stackedWidget->setCurrentIndex(index + 1);
        updateVisibility();
    } else {
        QApplication::quit();
    }
}

bool Widget::openDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QCoreApplication::applicationDirPath() + "/../../mydatabase.db";
    db.setDatabaseName(dbPath);
    qDebug() << "Opening DB at:" << dbPath;
    return db.open();
}

void Widget::createTable()
{
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "barcode TEXT, "
               "testNumber INTEGER, "
               "backlight_voltage TEXT, "
               "backlight_current TEXT, "
               "touch_test TEXT)");

    // Check if testNumber column is missing in existing tables and add it
    QSqlQuery checkColumnQuery(db);
    checkColumnQuery.exec("PRAGMA table_info(records)");
    bool hasTestNumber = false;
    while (checkColumnQuery.next()) {
        if (checkColumnQuery.value(1).toString() == "testNumber") {
            hasTestNumber = true;
            break;
        }
    }

    if (!hasTestNumber) {
        QSqlQuery alterQuery(db);
        alterQuery.exec("ALTER TABLE records ADD COLUMN testNumber INTEGER");
    }
}
void Widget::saveTextToFile(const QString &label, const QString &text)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/../../data.txt";
    QFile file(filePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << label << ": " << text << "\n";
        file.close();
    }
}

bool Widget::insertBarcode(const QString &)
{
    // This function is no longer used.
    return false;
}

bool Widget::updateRecordColumn(const QString &column, const QString &value)
{
    if (currentRecordId == -1) return false;

    QSqlQuery query(db);
    QString sql = QString("UPDATE records SET %1 = :value WHERE id = :id").arg(column);
    query.prepare(sql);
    query.bindValue(":value", value);
    query.bindValue(":id", currentRecordId);
    return query.exec();
}

void Widget::updateVisibility()
{
    int index = ui->stackedWidget->currentIndex();
    ui->nextButton->setVisible(index != 0);
    ui->ScannedBarCode_2->setVisible(index >= 1);

    if (index == 1 && currentBarcode.isEmpty()) {
        ui->ScannedBarCode_2->setText("Please scan the barcode");
    }
}
*/
#include "widget.h"
#include "ui_widget.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QApplication>
#include <QIntValidator>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    QIntValidator *validator = new QIntValidator(0, 499, this);
    ui->voltLineEdit->setValidator(validator);
    ui->currentLineEdit->setValidator(validator);

    ui->touchComboBox->clear();
    ui->touchComboBox->addItem("Select...");
    ui->touchComboBox->addItem("pass");
    ui->touchComboBox->addItem("fail");
    ui->touchComboBox->setCurrentIndex(0);

    connect(ui->barcodeLineEdit, &QLineEdit::returnPressed, this, &Widget::handleBarcodeScanned);
    connect(ui->voltLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->currentLineEdit, &QLineEdit::returnPressed, this, &Widget::handleAnyLineEditReturnPressed);
    connect(ui->nextButton, &QPushButton::clicked, this, &Widget::onNextButtonClicked);
    connect(ui->NewtestpushButton, &QPushButton::clicked, this, &Widget::onNewTestButtonClicked);
    connect(ui->reportpushButton, &QPushButton::clicked, this, &Widget::onReportButtonClicked);
    connect(ui->generatepushButton, &QPushButton::clicked, this, &Widget::onGenerateButtonClicked);

    if (!openDatabase()) {
        QMessageBox::critical(this, "Database", "Failed to open database.");
    } else {
        createTable();
    }

    updateVisibility();
}

Widget::~Widget()
{
    if (db.isOpen()) db.close();
    delete ui;
}

void Widget::onNewTestButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->barcodeLineEdit->clear();
    ui->barcodeLineEdit->setVisible(true);
    ui->barcodeLineEdit->setFocus();
    currentBarcode.clear();
    currentRecordId = -1;
    ui->ScannedBarCode_2->setText("Please scan the barcode");
    updateVisibility();
}

void Widget::onReportButtonClicked()
{
    ui->stackedWidget->setCurrentIndex(5);
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
    query.prepare("SELECT testNumber, backlight_voltage, backlight_current, touch_test "
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

    query.previous(); // rewind

    QString filePath = QCoreApplication::applicationDirPath() + "/../../report_" + barcode + ".txt";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error", "Unable to write report file.");
        return;
    }

    QTextStream out(&file);
    out << "Report for Barcode: " << barcode << "\n\n";

    while (query.next()) {
        int testNum = query.value(0).toInt();
        QString volt = query.value(1).toString();
        QString curr = query.value(2).toString();
        QString touch = query.value(3).toString();

        out << "Test #" << testNum << "\n";
        out << "Voltage: " << volt << "\n";
        out << "Current: " << curr << "\n";
        out << "Touch Test: " << touch << "\n\n";
    }

    out << "--- End of Report ---\n";
    file.close();

    QMessageBox::information(this, "Report Generated", "Report saved as report_" + barcode + ".txt");
    // Proceed to finish screen after report
    ui->stackedWidget->setCurrentIndex(4);
    updateVisibility();
}

void Widget::handleBarcodeScanned()
{
    QString barcode = ui->barcodeLineEdit->text().trimmed();
    if (barcode.isEmpty()) return;

    currentBarcode = barcode;

    // determine testNumber
    int testNumber = 1;
    QSqlQuery countQuery(db);
    countQuery.prepare("SELECT MAX(testNumber) FROM records WHERE barcode = :barcode");
    countQuery.bindValue(":barcode", barcode);
    if (countQuery.exec() && countQuery.next()) {
        testNumber = countQuery.value(0).toInt() + 1;
    }

    ui->ScannedBarCode_2->setText("Scanned: " + barcode);
    saveTextToFile("Barcode", barcode);
    saveTextToFile("TestNumber", QString::number(testNumber));

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO records (barcode, testNumber) VALUES (:barcode, :testNumber)");
    insertQuery.bindValue(":barcode", barcode);
    insertQuery.bindValue(":testNumber", testNumber);

    if (!insertQuery.exec()) {
        qDebug() << "Insert failed:" << insertQuery.lastError().text();
        return;
    }

    QSqlQuery selectQuery(db);
    selectQuery.prepare("SELECT id FROM records WHERE barcode = :barcode AND testNumber = :testNumber ORDER BY id DESC LIMIT 1");
    selectQuery.bindValue(":barcode", barcode);
    selectQuery.bindValue(":testNumber", testNumber);
    if (selectQuery.exec() && selectQuery.next()) {
        currentRecordId = selectQuery.value(0).toInt();
    }

    ui->barcodeLineEdit->clear();
    ui->barcodeLineEdit->setVisible(false);
}

void Widget::handleAnyLineEditReturnPressed()
{
    QLineEdit *line = qobject_cast<QLineEdit *>(sender());
    if (!line || currentRecordId == -1) return;

    QString text = line->text().trimmed();
    if (text.isEmpty()) return;

    QString label, column;
    if (line == ui->voltLineEdit || line == ui->currentLineEdit) {
        bool ok;
        int val = text.toInt(&ok);
        if (!ok || val >= 500) {
            QMessageBox::warning(this, "Invalid Input", "Please enter a number less than 500.");
            line->clear();
            return;
        }
    }

    if (line == ui->voltLineEdit) {
        label = "Volt";
        column = "backlight_voltage";
    } else if (line == ui->currentLineEdit) {
        label = "Current";
        column = "backlight_current";
    }

    saveTextToFile(label, text);
    updateRecordColumn(column, text);
    line->clear();
}

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

        bool voltOk, currentOk;
        int voltVal = volt.toInt(&voltOk);
        int currVal = current.toInt(&currentOk);

        if (!voltOk || voltVal >= 500 || !currentOk || currVal >= 500) {
            QMessageBox::warning(this, "Input Error", "Please enter valid values under 500.");
            return;
        }

        saveTextToFile("Volt", volt);
        saveTextToFile("Current", current);
        updateRecordColumn("backlight_voltage", volt);
        updateRecordColumn("backlight_current", current);

        ui->voltLineEdit->clear();
        ui->currentLineEdit->clear();
    }

    if (index == 3) {
        QString touch = ui->touchComboBox->currentText().trimmed().toLower();
        if (touch != "pass" && touch != "fail") {
            QMessageBox::warning(this, "Input Error", "Please select pass/fail.");
            return;
        }

        saveTextToFile("Touch", touch);
        updateRecordColumn("touch_test", touch);
    }

    if (index == 4) {
        QApplication::quit();
    } else {
        ui->stackedWidget->setCurrentIndex(index + 1);
        updateVisibility();
    }
}

bool Widget::openDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString path = QCoreApplication::applicationDirPath() + "/../../mydatabase.db";
    db.setDatabaseName(path);
    return db.open();
}

void Widget::createTable()
{
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "barcode TEXT, "
               "testNumber INTEGER, "
               "backlight_voltage TEXT, "
               "backlight_current TEXT, "
               "touch_test TEXT)");

    // Add testNumber column if missing
    QSqlQuery checkQuery("PRAGMA table_info(records)");
    bool hasTestNumber = false;
    while (checkQuery.next()) {
        if (checkQuery.value(1).toString() == "testNumber") {
            hasTestNumber = true;
            break;
        }
    }

    if (!hasTestNumber) {
        QSqlQuery alter(db);
        alter.exec("ALTER TABLE records ADD COLUMN testNumber INTEGER");
    }
}

void Widget::saveTextToFile(const QString &label, const QString &text)
{
    QString path = QCoreApplication::applicationDirPath() + "/../../data.txt";
    QFile file(path);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << label << ": " << text << "\n";
        file.close();
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
    return query.exec();
}

void Widget::updateVisibility()
{
    int index = ui->stackedWidget->currentIndex();
    ui->nextButton->setVisible(index != 0);
    ui->ScannedBarCode_2->setVisible(index >= 1);
    if (index == 1 && currentBarcode.isEmpty())
        ui->ScannedBarCode_2->setText("Please scan the barcode");
}
