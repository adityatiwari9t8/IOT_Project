# Architecture

## Module responsibilities

| Module | Files | Responsibility |
|---|---|---|
| `config.h` | `include/config.h` | Every pin number and timing constant, in one place. Nothing else hardcodes a GPIO number. |
| `secrets.h` | `include/secrets.h` (gitignored) | Wi-Fi credentials, the Apps Script URL, and the shared API key. Never committed. |
| `StudentDatabase` | `include/`/`src/StudentDatabase.*` | The in-memory roster. Pure data + lookup logic — no hardware calls, no networking. |
| `DisplayUI` | `include/`/`src/DisplayUI.*` | Wraps the SSD1306 behind one `showMessage(title, l1, l2, l3)` call. |
| `FeedbackIndicators` | `include/`/`src/FeedbackIndicators.*` | Owns the 3 LEDs + buzzer. `success()` / `error()` / `networkBusy()`. |
| `AttendanceLogger` | `include/`/`src/AttendanceLogger.*` | Wi-Fi connect (with timeout) + HTTPS GET to the Apps Script. Returns a `SyncResult` enum instead of leaking HTTP codes everywhere. |
| `main.cpp` | `src/main.cpp` | Wires the above together. Reads a card, asks `StudentDatabase` what it means, tells `DisplayUI`/`FeedbackIndicators`/`AttendanceLogger` what to do. No pin numbers, no HTTP calls, no display primitives — just orchestration. Also owns the boot-time reset-button check, since it's a one-shot setup concern rather than something any single module needs repeatedly. |

This split exists so each piece can be reasoned about (and marked!) on its
own: a grader can read `StudentDatabase.cpp` in isolation and verify the
lookup logic is correct without also reading OLED or Wi-Fi code.

## Request flow (happy path)

```
Card tapped
   -> main.cpp reads UID via MFRC522
   -> StudentDatabase.lookup(uid) -> NEW_ENTRY
   -> DisplayUI.showMessage("WELCOME!", name, ...)
   -> AttendanceLogger.logAttendance(uid, name)
        -> WiFiClientSecure + HTTPClient GET to Apps Script, with ?key=API_KEY
        -> Apps Script checks the key, checks for a duplicate today,
           appends a row, returns "Success: ..."
   -> SyncResult::SUCCESS
   -> DisplayUI.showMessage("ATTENDANCE LOGGED", ...)
   -> FeedbackIndicators.success()   (green LED + beep)
   -> StudentDatabase.markLogged(index)
```

## Attendance now survives a reset

`Student.logged` is backed by the ESP32's NVS flash via the `Preferences`
library (see `StudentDatabase::begin()`/`markLogged()`), not just a RAM
array. A brownout, crash, or `ESP.restart()` mid-class no longer loses
who's already tapped in — `begin()` restores every student's saved state
on boot, keyed by their UID.

That state has to be cleared deliberately for a new session, which is
what the reset button (`PIN_BUTTON_RESET`, held for `RESET_HOLD_MS` during
boot) is for — see `main.cpp::setup()`. The Apps Script's
`isDuplicateToday_()` check is still there as a second, independent
backstop: even if someone forgets to hold the reset button between
classes, the sheet itself won't accumulate duplicate rows for the same
day.

## Why `WiFiClientSecure::setInsecure()`

Google's Apps Script endpoint is HTTPS-only. The ESP32 has no built-in,
auto-updating CA certificate store, and Google rotates its TLS certs
periodically, so pinning a certificate would break the project the next
time Google rotates. `setInsecure()` — skipping certificate-chain
validation — is the standard, documented approach for ESP32 projects
talking to Apps Script and is what this project uses. It's acceptable
here because the payload (a UID and a name) isn't sensitive and the
endpoint is protected by the `API_KEY` shared secret instead. For a
production system handling sensitive data you'd want `setCACert()` with
Google's root CA, refreshed periodically.
