# Qt Device Test Application

This Qt-based application is designed for testing and recording hardware device data (e.g., display units). It supports scanning barcodes, entering voltage and current measurements, running a touch test, and generating detailed test reports.

---

## Features

- **5-Step Workflow Using QStackedWidget**:
  1. **Welcome Page**  
     - Start a new test or generate a report
  2. **Barcode Page**  
     - Scan and save barcode
  3. **Voltage & Current Page**  
     - Enter backlight voltage and current values
  4. **Touch Test Page**  
     - Select "pass" or "fail"
  5. **Finish Page**  
     - Indicates end of test flow

- **Barcode Scanner Support**:  
  Captures scanned barcode from a hidden `QLineEdit`.

- **Data Storage**:
  - **SQLite Database (`mydatabase.db`)**
    - Stores each test with:  
      `barcode`, `testNumber`, `backlight_voltage`, `backlight_current`, `touch_test`
  - **Text File Logging (`data.txt`)**
    - Appends values with labels for backup/reference

- **Auto Test Numbering**:
  - Each time a barcode is scanned, a new `testNumber` is generated automatically based on prior entries.

- **Report Generation**:
  - Enter a barcode to generate a text report (`report_<barcode>.txt`)
  - Lists all test entries for that barcode
  - Saved in the app directory (2 levels above)

---

## Dependencies

- Qt 5 or Qt 6
- SQLite (included with Qt by default)

---

## UI Components

| Widget                     | Purpose                             |
|---------------------------|-------------------------------------|
| `barcodeLineEdit`         | Hidden input for scanned barcode    |
| `ScannedBarCode_2`        | Displays scanned barcode            |
| `voltLineEdit`            | Input for voltage (max 499)         |
| `currentLineEdit`         | Input for current (max 499)         |
| `touchComboBox`           | Select "pass" or "fail"             |
| `nextButton`              | Proceeds to the next test step      |
| `NewtestpushButton`       | Starts a new test                   |
| `reportpushButton`        | Goes to report search screen        |
| `generatepushButton`      | Generates report from barcode input |
| `reportBarCodelineEdit`   | Enter barcode to generate report    |

---

## File Structure

project_root/
├── build/
│ └── mydatabase.db # SQLite database file
├── data.txt # Log of test values (text file)
├── report_<barcode>.txt # Auto-generated reports
├── widget.h # Header file
├── widget.cpp # Main application logic
└── main.cpp # Entry point


---

## Running the App

1. **Open in Qt Creator**
2. **Build and Run**
3. **From the Welcome Page**:
   - Click **"New Test"** to start the testing process
   - Click **"Generate Report"** to search test records by barcode

---

## Notes

- The database and reports are saved **2 directories above** the app binary location.
- Only values under **500** are accepted for voltage and current.
- Touch test must be either **"pass"** or **"fail"**.

---


