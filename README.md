# 🎓 ESP32 Smart Attendance System

![C++](https://img.shields.io/badge/Language-C++-blue.svg)
![Platform](https://img.shields.io/badge/Platform-ESP32-lightgrey.svg)
![Framework](https://img.shields.io/badge/Framework-Arduino-00979D.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

An RFID-based classroom attendance system built on an ESP32, simulated
entirely in [Wokwi](https://wokwi.com/), with attendance logged in real
time to a Google Sheet. Built for a B.Tech CSE final-semester IoT
project.

> **🌍 SDG alignment:** UN Sustainable Development Goal 4 — Quality
> Education — by digitizing attendance tracking to reduce administrative
> overhead and give faculty real-time visibility into classroom presence.

---

## Table of contents

- [Features](#-features)
- [Hardware](#-hardware-wokwi-simulated)
- [Software stack](#-software-stack)
- [Wiring](#-wiring)
- [Project structure](#-project-structure)
- [Getting started](#-getting-started)
- [Deploying the Google Sheets backend](#-deploying-the-google-sheets-backend)
- [How it works](#-how-it-works)
- [Testing it in Wokwi](#-testing-it-in-wokwi)
- [Known limitations](#-known-limitations)
- [Future improvements](#-future-improvements)
- [License](#-license)

---

## ✨ Features

- **Tap-to-log architecture** — reads RFID cards over SPI, validated
  against an onboard student roster.
- **Persistent attendance (NVS flash)** — each student's "logged" state
  is saved to the ESP32's non-volatile storage via the `Preferences`
  library, not just held in RAM. A reset or power loss mid-class doesn't
  wipe who's already tapped in.
- **Manual reset button** — hold a dedicated button for 3 seconds during
  boot to wipe saved attendance and start a clean session (e.g. the next
  class). Release early to cancel.
- **Google Cloud integration** — writes to a Google Sheet via a Google
  Apps Script Web App over HTTPS, protected by a shared API key.
- **Graceful offline mode** — if Wi-Fi is unavailable, the device times
  out safely after 15s instead of hanging, and still grants entry
  locally.
- **Visual + audio feedback** — an OLED status screen for every state
  (booting, connecting, ready, welcome, denied, duplicate, synced,
  offline), paired with 3 LEDs and a buzzer.
- **Server-side duplicate protection** — the Apps Script backend
  independently checks for a duplicate UID logged earlier the same day,
  as a second backstop beyond the device's own state.
- **Secrets kept out of version control** — Wi-Fi credentials, the Apps
  Script URL, and the API key live in a gitignored `secrets.h`.

## 🛠 Hardware (Wokwi simulated)

| Component | Protocol / Pins | Purpose |
| :--- | :--- | :--- |
| ESP32 DevKit-C V4 | — | Main controller, Wi-Fi |
| MFRC522 RFID reader | SPI — SS `5`, RST `27`, SCK `18`, MOSI `23`, MISO `19` | Reads student ID cards |
| SSD1306 128×64 OLED | I2C — SDA `21`, SCL `22` | Status display |
| LEDs | GPIO `13` (green), `12` (red), `14` (blue) | Accepted / rejected / busy |
| Piezo buzzer | GPIO `15` | Audible success/error tone |
| Reset pushbutton | GPIO `4` | Hold during boot to wipe saved attendance |

## 💻 Software stack

- **Firmware:** Arduino framework (C++), built with **PlatformIO**
- **Simulator:** Wokwi VS Code extension
- **Persistence:** ESP32 `Preferences` library (NVS flash)
- **Backend:** Google Apps Script Web App (`doGet`), writing to a Google
  Sheet
- **Libraries:** `MFRC522`, `Adafruit SSD1306`, `Adafruit GFX` — see
  `platformio.ini`

## 🔌 Wiring

Matches `diagram.json` exactly — if you ever change a pin, update both
`include/config.h` and `diagram.json` together.

| Signal | ESP32 GPIO |
|---|---|
| LED Green | 13 |
| LED Red | 12 |
| LED Blue | 14 |
| Buzzer | 15 |
| Reset button | 4 |
| RFID SDA (SS) | 5 |
| RFID SCK | 18 |
| RFID MOSI | 23 |
| RFID MISO | 19 |
| RFID RST | 27 |
| RFID 3.3V | 3V3 |
| OLED SDA | 21 |
| OLED SCL | 22 |
| OLED VCC | 5V |

> **Note:** RFID `RST` is on GPIO27, *not* GPIO21 — GPIO21 is already
> used by the OLED's I2C SDA line. An earlier revision of this project
> had both on GPIO21, a real wiring conflict that's fixed here.

## 📁 Project structure

```
IOT_Project/
├── backend/
│   └── AppsScript.gs           # Google Sheets Web App backend
├── docs/
│   └── ARCHITECTURE.md         # Module breakdown + data flow
├── include/
│   ├── config.h                # Pins & timing constants (non-secret)
│   ├── secrets.example.h       # Template — copy to secrets.h
│   ├── secrets.h               # Your real values — gitignored
│   ├── StudentDatabase.h       # NVS-backed roster + lookup logic
│   ├── DisplayUI.h
│   ├── FeedbackIndicators.h
│   └── AttendanceLogger.h
├── src/
│   ├── main.cpp                # Orchestration + boot-time reset check
│   ├── StudentDatabase.cpp
│   ├── DisplayUI.cpp
│   ├── FeedbackIndicators.cpp
│   └── AttendanceLogger.cpp
├── diagram.json                # Wokwi circuit diagram
├── wokwi.toml                  # Wokwi <-> PlatformIO build linkage
├── platformio.ini
├── CHANGELOG.md
├── LICENSE
└── README.md
```

## 🚀 Getting started

**Prerequisites:** VS Code, the PlatformIO extension, the Wokwi
extension, and a (free) Wokwi account for simulation.

1. Clone the repo and open it in VS Code.
2. Create your local secrets file:
   ```bash
   cp include/secrets.example.h include/secrets.h
   ```
3. Fill in `include/secrets.h`:
   - `WIFI_SSID` / `WIFI_PASSWORD` — leave as `Wokwi-GUEST` / `""` for
     simulation.
   - `GOOGLE_SCRIPT_URL` — from the deployment steps below.
   - `API_KEY` — any random string; it must match `API_KEY` in
     `backend/AppsScript.gs`.
4. Build with PlatformIO (checkmark icon, or `pio run`).
5. Press **F1 → "Wokwi: Start Simulator"** (or the Wokwi sidebar icon).

## ☁️ Deploying the Google Sheets backend

1. Create a new Google Sheet.
2. **Extensions → Apps Script**, delete the boilerplate, and paste in
   the contents of `backend/AppsScript.gs`.
3. Set `API_KEY` at the top of the script to the same string you put in
   `include/secrets.h`.
4. **Deploy → New deployment → Web app**:
   - Execute as: **Me**
   - Who has access: **Anyone**
5. Copy the `.../exec` URL into `GOOGLE_SCRIPT_URL` in
   `include/secrets.h`.
6. Any time you edit `AppsScript.gs` afterward, use **Deploy → Manage
   deployments → ✏️ → New version** — editing the script alone does not
   update the live URL.

## 🧠 How it works

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the full module
breakdown and request-flow diagram. Short version: `main.cpp` reads a
card UID, asks `StudentDatabase` whether it's unknown / already logged /
a new valid tap, shows the result via `DisplayUI` and
`FeedbackIndicators`, and — for a new valid tap — sends it to
`AttendanceLogger`, which does the HTTPS call to the Apps Script backend.
`StudentDatabase` itself is backed by NVS flash, so "already logged"
state isn't lost on a reset.

## 🧪 Testing it in Wokwi

In the running simulation, right-click the RFID reader part and choose
**"Simulate a Scan"**, then pick one of these UIDs (or type your own to
test the unknown-card path):

| UID | Student |
|---|---|
| `01020304` | Aisyah |
| `11223344` | Balqis |
| `55667788` | Danish |
| `AABBCCDD` | Farah |

**Test scenarios:**

1. **Happy path** — tap a valid card. OLED shows "WELCOME!", green LED
   flashes, logs to the sheet.
2. **Duplicate handling** — tap the same card again. Red LED flashes,
   OLED shows "ALREADY LOGGED".
3. **Unknown card** — tap an unregistered UID. Red LED, "ACCESS DENIED".
4. **Resilience test** — with a card already logged, trigger a soft
   reset (`ESP.restart()`, or the board's reset control in Wokwi).
   Tapping that same card again should still show "ALREADY LOGGED" —
   confirming the NVS-backed state survived the reset, not just RAM.
5. **Manual reset** — power-cycle the simulation while holding the reset
   button for 3+ seconds. OLED shows "ATTENDANCE RESET"; a previously
   logged card will now register as a fresh tap.

## ⚠️ Known limitations

- **NVS key length.** Each UID is used directly as the flash storage
  key, and ESP32 NVS keys are capped at 15 characters. The roster's
  8-character hex UIDs are safely under that limit — worth rechecking if
  the UID format ever changes.
- **No certificate pinning.** `WiFiClientSecure::setInsecure()` skips TLS
  certificate validation (see `docs/ARCHITECTURE.md` for why this is the
  standard approach for ESP32 + Apps Script, and what real cert pinning
  would look like).
- **Blocking `delay()` calls** for LED/buzzer feedback (500–600ms) and
  result screens (1.5–2s), plus a busy-wait loop while the reset button
  is held, mean the loop briefly can't read a second card or run other
  logic during those windows. Fine for a single-reader classroom demo; a
  higher-throughput system would use non-blocking `millis()` timers.
- **Fixed roster.** Students are hardcoded in `main.cpp`. Adding a
  student means reflashing.

## 🔭 Future improvements

- Move the roster to SPIFFS/LittleFS as a JSON file, or fetch it from
  the same Apps Script backend, so it's editable without reflashing.
- Parse the Apps Script's JSON response instead of only checking the
  HTTP status code, to surface duplicate/error messages on the OLED.
- Add a card **enrollment mode** (hold a button, tap an unknown card,
  type a name over serial) instead of a hardcoded roster.
- Non-blocking state machine instead of `delay()`-based timing, to
  support back-to-back rapid taps and a non-blocking reset-hold check.

## 📄 License

MIT — see [`LICENSE`](LICENSE).

---
*Developed for B.Tech CSE finals.*
