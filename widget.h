/*#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void handleBarcodeScanned();
    void onNextButtonClicked();
    void handleAnyLineEditReturnPressed();
    void onNewTestButtonClicked();

private:
    Ui::Widget *ui;
    QSqlDatabase db;
    QString currentBarcode;
    int currentRecordId = -1;

    bool openDatabase();
    void createTable();
    void saveTextToFile(const QString &label, const QString &text);
    bool insertBarcode(const QString &barcode);
    bool updateRecordColumn(const QString &column, const QString &value);
    void updateVisibility();
};

#endif // WIDGET_H
*/
/*#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void handleBarcodeScanned();
    void onNextButtonClicked();
    void handleAnyLineEditReturnPressed();
    void onNewTestButtonClicked();

private:
    Ui::Widget *ui;
    QSqlDatabase db;
    QString currentBarcode;
    int currentRecordId = -1;

    bool openDatabase();
    void createTable();
    void saveTextToFile(const QString &label, const QString &text);
    bool insertBarcode(const QString &barcode);
    bool updateRecordColumn(const QString &column, const QString &value);
    void updateVisibility();
};

#endif // WIDGET_H
*/
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void handleBarcodeScanned();
    void onNextButtonClicked();
    void handleAnyLineEditReturnPressed();
    void onNewTestButtonClicked();
    void onReportButtonClicked();         // From Welcome Page to Report Widget
    void onGenerateButtonClicked();       // When Generate Report is clicked

private:
    Ui::Widget *ui;
    QSqlDatabase db;
    QString currentBarcode;
    int currentRecordId = -1;

    bool openDatabase();
    void createTable();
    void saveTextToFile(const QString &label, const QString &text);
    bool insertBarcode(const QString &barcode);
    bool updateRecordColumn(const QString &column, const QString &value);
    void updateVisibility();
};

#endif // WIDGET_H
