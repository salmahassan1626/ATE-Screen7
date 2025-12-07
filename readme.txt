# Qt Display Testing Automation System

A complete desktop application developed in Qt for automating display module testing, barcode tracking, measurement collection, and generating detailed technical reports.
The system integrates hardware devices such as barcode scanners, Arduino current/voltage meters, and X-Rite/Calibrite color measurement devices.

This application is designed for production lines and R&D labs where display panels must undergo a series of validation tests. The system aims to:

Standardize test procedures
Log every test cycle
Ensure traceability using unique barcodes
Automate electrical measurements
Reduce operator input errors
Produce consistent technical reports

The program uses a QStackedWidget-based multi-page interface, guiding the operator through a fixed sequence of steps.
---------------------------------------

##  System Architecture
Main Components:

Qt Widgets application

SQLite database for long-term storage of test data

Arduino (USB Serial) for electrical measurements

Calibrite Profiler/X-Rite tool manually operated but linked to the workflow

Text-based report generator

Barcode scanner that acts as a keyboard input device

### 1. **Barcode Scanning**

* Uses a hidden `QLineEdit (barcodeLineEdit)` to capture barcode scanner input.
* The scanned barcode is immediately:

  * Displayed in `ScannedBarCode_2` label on test pages
  * Saved into the SQLite database as a new test row
  * Used as the unique ID for a test session

---

### 2. **SQLite Database Integration**

Database table: **records**

Columns:

* `id` (auto)
* `barcode`
* `testNumber`
* `backlight_voltage`
* `backlight_current`
* `power_watts`
* `touch_test`
* `rgb`
* `luminance`
* `contrast`

Each new barcode scan automatically creates:

```
barcode | testNumber | empty measurement fields
```

Measurements are updated column-by-column as the user progresses through the test steps.

Database file path:

```
<applicationDir>/../../mydatabase.db
```

---

### 3. **Arduino Integration**

* Communicates via **COM7** at 115200 baud.
* Sends command `"R"` to request readings.
* Arduino responds with voltage and current readings.
* Application averages readings and auto-fills:

  * Voltage
  * Current
  * Power (watts)

If Arduino is unavailable, the user may enter values manually.

---

### 4. **X-Rite Color Readings**

* User manually enters:

  * RGB value
  * Luminance
  * Contrast
* Values are immediately written into the database.

---

### 5. **Calibrite Profiler Integration**

* Application auto-launches:

```
C:\Program Files\calibrite PROFILER\calibrite PROFILER.exe
```

when entering the X-Rite measurement page.

---

### 6. **Report Generation**

* The user enters a barcode on the "Report" page.
* The application fetches all test cycles for that barcode.
* Generates a text report:

```
/Reports/report_<barcode>.txt
```

Each entry includes:

* Voltage
* Current
* Power
* Touch Test
* RGB
* Luminance
* Contrast

---

## Directory Structure

```
project/
│── widget.cpp
│── widget.h
│── ui_widget.h
│── mydatabase.db
│── Reports/
│     └── report_<barcode>.txt
│── readme.txt
```

---

## How the Application Works (Flow)

1. Start App → Welcome Page
2. User clicks **New Test**
3. Scan barcode → new DB row created
4. Arduino auto-readings collected
5. User confirms values → saved automatically
6. X-Rite readings entered → saved
7. Finish page → application closes

User may also select **Report** from the home page to generate historical test results.

---

## Requirements

### Hardware:

* Arduino (connected on COM7)
* Barcode Scanner
* X-Rite calibration tool (optional)
* Windows PC

### Software:

* Qt 
* SQLite (built-in)
* Calibrite Profiler installed in Program Files

---

## Known Limitations

* Arduino communication requires COM7 (hardcoded).
* X-Rite measurements are manual, not automated.
* Database path is relative to executable location.

---------------------



