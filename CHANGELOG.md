# Changelog

## Unreleased — NVS persistence & reset button

**Reliability**
- Attendance state now survives a reset or power loss mid-class.
  `StudentDatabase` is backed by the ESP32's NVS flash via the
  `Preferences` library instead of a RAM-only array: `begin()` restores
  each student's saved `logged` state on boot, `markLogged()` writes
  through to flash immediately.
- Added a physical reset button (`PIN_BUTTON_RESET`, GPIO4) — hold it
  through boot for 3s to wipe the saved attendance and start a fresh
  session (e.g. the next class). Release early to cancel. Wired as a
  real `wokwi-pushbutton` part rather than the devkit's silkscreen BOOT
  button, since we couldn't confirm that's clickable in Wokwi's UI.
- The Apps Script's server-side duplicate check remains in place as a
  second backstop, independent of whether the reset button gets
  pressed between sessions.

**Note on ESP32 NVS key limits:** each student's UID is used directly as
the NVS storage key, and NVS keys are capped at 15 characters. Our UIDs
(8 hex chars) are safely under that; keep it in mind if the roster ever
moves to a longer UID format.

## Unreleased — Code review & hardening pass

**Bug fixes**
- Fixed a real pin conflict: RFID `RST` was wired to GPIO21, which is also
  the OLED's I2C SDA line. Moved to GPIO27 in both `main.cpp` and
  `diagram.json`.
- HTTPS requests to the Apps Script now use `WiFiClientSecure`
  explicitly. The previous code called `http.begin(url)` with an
  `https://` URL and no secure client configuration, which is not
  guaranteed to negotiate TLS correctly on all ESP32 Arduino core
  versions.
- Wi-Fi connect no longer blocks forever — added a 15s timeout with a
  graceful fallback to offline mode.
- `students[i].logged` is now set *after* the cloud sync attempt
  completes, not before, so a mid-request crash/reset can't leave a
  student marked present with nothing actually sent.

**Security**
- Removed the hardcoded Wi-Fi/Apps-Script URL from `main.cpp`. Real
  values now live in `include/secrets.h`, which is gitignored;
  `include/secrets.example.h` is the committed template.
- Added a shared-secret `API_KEY` check on both firmware and backend, so
  the Apps Script URL can't be used to write junk data if it leaks or is
  guessed.
- Backend now rejects requests with a missing/invalid key or missing
  `uid`/`name` instead of silently writing incomplete rows.

**Reliability**
- Backend now checks for a duplicate UID logged earlier the same day and
  skips re-appending a row, as a server-side backstop for the fact that
  the device's own "already logged" state resets on every reboot.
- HTTP success is now checked as a `200–299` range instead of `> 0`
  (which also treated negative `HTTPClient` error codes as "success").

**Structure**
- Split the single 230-line `main.cpp` into `StudentDatabase`,
  `DisplayUI`, `FeedbackIndicators`, and `AttendanceLogger` modules with
  matching headers in `include/`. `main.cpp` is now orchestration only.
- Added `docs/ARCHITECTURE.md`, this changelog, `LICENSE` (MIT), and a
  full project `README.md`.
- Moved the backend into `backend/AppsScript.gs` so it's version
  controlled alongside the firmware instead of living only in the Apps
  Script editor.
