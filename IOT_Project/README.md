# ESP32 Smart Attendance System

An RFID-based classroom attendance system built on an ESP32, simulated
entirely in [Wokwi](https://wokwi.com/), with attendance logged to a
Google Sheet in real time. Built for a B.Tech CSE final-semester IoT
project.

> **SDG alignment:** UN Sustainable Development Goal 4 — Quality
> Education — by digitizing attendance tracking to reduce administrative
> overhead and give faculty real-time visibility into classroom presence.

---

## Table of contents

- [Features](#features)
- [Hardware](#hardware-simulated-in-wokwi)
- [Software stack](#software-stack)
- [Wiring](#wiring)
- [Project structure](#project-structure)
- [Getting started](#getting-started)
- [Deploying the Google Sheets backend](#deploying-the-google-sheets-backend)
- [How it works](#how-it-works)
- [Testing it in Wokwi](#testing-it-in-wokwi)
- [Known limitations](#known-limitations)
- [Future improvements](#future-improvements)
- [License](#license)

---

## Features

- Tap-to-log attendance with an RFID card, matched against an onboard
  student roster.
- OLED status screen for every state: booting, connecting, ready,
  welcome, denied, duplicate, synced, offline.
- Visual + audible feedback via 3 LEDs (green/red/blue) and a piezo
  buzzer — no need to watch the serial monitor to know what happened.
- Attendance rows written to a Google Sheet via a Google Apps Script Web
  App, over HTTPS, protected by a shared API key.
- Graceful offline mode: if Wi-Fi is unavailable, the device times out
  after 15s instead of hanging, and still grants entry locally.
- Server-side duplicate protection, independent of the device's own
  in-memory "already logged" state.
- Secrets (Wi-Fi credentials, Apps Script URL, API key) kept out of
  version control.

## Hardware (simulated in Wokwi)

| Component | Purpose |
|---|---|
| ESP32 DevKit-C V4 | Main controller, Wi-Fi |
| MFRC522 RFID reader | Reads student ID cards over SPI |
| SSD1306 128×64 OLED | Status display over I2C |
| LED — Green | Attendance accepted |
| LED — Red | Rejected (unknown card / duplicate tap) |
| LED — Blue | Busy (Wi-Fi connecting / cloud sync in progress) |
| Piezo buzzer | Audible success/error tone |

## Software stack

- **Firmware:** Arduino framework (C++), built with **PlatformIO**
- **Simulator:** Wokwi VS Code extension
- **Backend:** Google Apps Script Web App (`doGet`), writing to a Google
  Sheet
- **Libraries:** `MFRC522`, `Adafruit SSD1306`, `Adafruit GFX` — see
  `platformio.ini`

## Wiring

Matches `diagram.json` exactly — if you ever change a pin, update both
`include/config.h` and `diagram.json` together.

| Signal | ESP32 GPIO |
|---|---|
| LED Green | 13 |
| LED Red | 12 |
| LED Blue | 14 |
| Buzzer | 15 |
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
> had both on GPIO21, which is a real wiring conflict; it's fixed here.

## Project structure

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
│   ├── StudentDatabase.h
│   ├── DisplayUI.h
│   ├── FeedbackIndicators.h
│   └── AttendanceLogger.h
├── src/
│   ├── main.cpp                # Orchestration only
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

## Getting started

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

## Deploying the Google Sheets backend

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

## How it works

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the full module
breakdown. Short version: `main.cpp` reads a card UID, asks
`StudentDatabase` whether it's unknown / already logged / a new valid
tap, shows the result via `DisplayUI` and `FeedbackIndicators`, and — for
a new valid tap — sends it to `AttendanceLogger`, which does the HTTPS
call to the Apps Script backend.

## Testing it in Wokwi

In the running simulation, right-click the RFID reader part and choose
**"Simulate a Scan"**, then pick one of these UIDs (or type your own to
test the unknown-card path):

| UID | Student |
|---|---|
| `01020304` | Aisyah |
| `11223344` | Balqis |
| `55667788` | Danish |
| `AABBCCDD` | Farah |

Try each of these to demo the full state machine: a valid card (green,
logged), the same card again (red, duplicate), an unlisted UID (red,
unknown), and disconnecting the simulated network to show offline mode.

## Known limitations

- **Attendance state is in-memory only.** `Student.logged` resets on
  every reboot. The Apps Script backend has its own duplicate check as a
  backstop, but the device's own "already logged" screen will reset. A
  real deployment would persist this with ESP32 `Preferences`/NVS.
- **No certificate pinning.** `WiFiClientSecure::setInsecure()` skips TLS
  certificate validation (see `docs/ARCHITECTURE.md` for why this is the
  standard approach for ESP32 + Apps Script, and what real cert pinning
  would look like).
- **Blocking `delay()` calls** for LED/buzzer feedback (500–600ms) and
  result screens (1.5–2s) mean the loop briefly can't read a second card
  during those windows. Fine for a single-reader classroom demo; a
  higher-throughput system would use non-blocking `millis()` timers.
- **Fixed roster.** Students are hardcoded in `main.cpp`. Adding a
  student means reflashing.

## Future improvements

- Persist attendance state (and last-flashed date) in NVS so a reboot
  mid-class doesn't reset "already logged" locally.
- Move the roster to SPIFFS/LittleFS as a JSON file, or fetch it from
  the same Apps Script backend, so it's editable without reflashing.
- Parse the Apps Script's JSON response instead of only checking the
  HTTP status code, to surface duplicate/error messages on the OLED.
- Add a card **enrollment mode** (hold a button, tap an unknown card,
  type a name over serial) instead of a hardcoded roster.
- Non-blocking state machine instead of `delay()`-based timing, to
  support back-to-back rapid taps.

## License

MIT — see [`LICENSE`](LICENSE).
