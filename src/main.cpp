// =====================================================================
// ESP32 Smart Attendance System
//
// Tap an RFID card -> looked up against an in-memory roster -> result
// shown on the OLED + LEDs/buzzer -> logged to a Google Sheet over
// HTTPS. See README.md for the full write-up, wiring table, and setup
// steps; see docs/ARCHITECTURE.md for how the pieces below fit together.
// =====================================================================

#include <MFRC522.h>
#include <SPI.h>

#include "AttendanceLogger.h"
#include "DisplayUI.h"
#include "FeedbackIndicators.h"
#include "StudentDatabase.h"
#include "config.h"
#include "secrets.h"  // for WIFI_SSID, shown on the "Connecting..." screen

// ---------------------------------------------------------------------
// Roster — Wokwi's virtual RFID reader accepts any of these preset UIDs.
// Tap "Simulate a Scan" in the RFID part's context menu and pick a UID
// to test each one, including UIDs NOT on this list (unknown-card path).
// ---------------------------------------------------------------------
Student roster[] = {
    {"01020304", "Aisyah", false},
    {"11223344", "Balqis", false},
    {"55667788", "Danish", false},
    {"AABBCCDD", "Farah", false},
};
const int ROSTER_SIZE = sizeof(roster) / sizeof(roster[0]);

MFRC522 rfid(PIN_RFID_SS, PIN_RFID_RST);
DisplayUI oled;
FeedbackIndicators feedback;
AttendanceLogger cloud;
StudentDatabase students(roster, ROSTER_SIZE);

// Reads the UID from a freshly-detected card as an uppercase hex string,
// e.g. {0x01, 0x02, 0x03, 0x04} -> "01020304". This is the same format
// used in the `roster` table above.
String readCardUID() {
  String uid;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += '0';
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void setup() {
  Serial.begin(115200);
  feedback.begin();
  students.begin();  // load any attendance saved to flash before the crash/reset

  pinMode(PIN_BUTTON_RESET, INPUT_PULLUP);

  // The OLED must be initialized before any showMessage() call — the
  // Adafruit_SSD1306 driver allocates its frame buffer inside begin().
  // Calling showMessage() first was a real bug caught during review.
  if (!oled.begin()) {
    // Can't show an error on-screen if the screen itself is the
    // problem — fall back to the serial monitor and halt, since the
    // rest of the UI depends entirely on this display.
    Serial.println("[FATAL] SSD1306 not found. Check wiring/I2C address.");
    for (;;) delay(1000);
  }

  // Hold the reset button through boot to wipe today's attendance —
  // e.g. at the start of a new class. Release early to cancel.
  if (digitalRead(PIN_BUTTON_RESET) == LOW) {
    oled.showMessage("RESET ATTENDANCE", "Keep holding...", "Release to cancel", "");
    unsigned long pressStart = millis();
    bool confirmed = false;
    while (digitalRead(PIN_BUTTON_RESET) == LOW) {
      if (millis() - pressStart >= RESET_HOLD_MS) {
        confirmed = true;
        break;
      }
    }
    if (confirmed) {
      students.resetAttendance();
      Serial.println("[SETUP] Attendance reset via button hold.");
      oled.showMessage("ATTENDANCE RESET", "All records wiped", "Starting fresh", "session");
      feedback.success();
      delay(1500);
    } else {
      Serial.println("[SETUP] Reset cancelled (released early).");
      oled.showMessage("RESET CANCELLED", "Released too soon", "", "");
      delay(1000);
    }
  }

  oled.showMessage("SMART ATTENDANCE", "Booting System...", "Please wait", "");

  SPI.begin();
  rfid.PCD_Init();

  feedback.networkBusy(true);
  oled.showMessage("WI-FI CONNECTION", "Connecting to:", WIFI_SSID, "");
  bool wifiOk = cloud.connect();
  feedback.networkBusy(false);

  if (wifiOk) {
    Serial.println("\n[SETUP] Wi-Fi connected.");
    feedback.wifiConnected();
  } else {
    Serial.println("\n[SETUP] Wi-Fi unavailable — continuing offline.");
  }

  oled.showReady(wifiOk);
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = readCardUID();
  Serial.print("\n[SCAN] UID: ");
  Serial.println(uid);

  int index;
  LookupResult result = students.lookup(uid, index);

  if (result == LookupResult::UNKNOWN_CARD) {
    oled.showMessage("ACCESS DENIED", "Unknown Card!", "UID: " + uid,
                      "Not Registered");
    feedback.error();
    delay(RESULT_DISPLAY_MS);
    oled.showReady(cloud.isConnected());
    rfid.PICC_HaltA();
    return;
  }

  if (result == LookupResult::ALREADY_LOGGED) {
    oled.showMessage("ALREADY LOGGED", students.at(index).name,
                      "Status: Present", "Duplicate scan");
    feedback.error();
    delay(RESULT_DISPLAY_MS);
    oled.showReady(cloud.isConnected());
    rfid.PICC_HaltA();
    return;
  }

  // LookupResult::NEW_ENTRY — first tap this session for a known student.
  const String& name = students.at(index).name;
  oled.showMessage("WELCOME!", name,
                    cloud.isConnected() ? "Syncing Cloud..." : "Offline attendance",
                    "");

  feedback.networkBusy(true);
  int httpCode = 0;
  SyncResult sync = cloud.logAttendance(uid, name, httpCode);
  feedback.networkBusy(false);

  switch (sync) {
    case SyncResult::SUCCESS:
      Serial.println("[SYNC] Logged to Google Sheets.");
      oled.showMessage("ATTENDANCE LOGGED", name, "Status: On Time",
                        "Cloud: Synced");
      feedback.success();
      break;
    case SyncResult::HTTP_ERROR:
      Serial.printf("[SYNC] HTTP error: %d\n", httpCode);
      oled.showMessage("CLOUD ERROR", name, "Recorded Locally",
                        "Err: " + String(httpCode));
      feedback.success();  // still grant entry locally
      break;
    case SyncResult::OFFLINE:
      Serial.println("[SYNC] Skipped — no Wi-Fi.");
      oled.showMessage("ATTENDANCE SAVED", name, "Recorded Locally",
                        "Cloud: Offline");
      feedback.success();
      break;
  }

  // Mark present only after the sync attempt, so a reset mid-request
  // can't leave a student marked present with nothing actually sent.
  students.markLogged(index);

  delay(SUCCESS_DISPLAY_MS);
  oled.showReady(cloud.isConnected());
  rfid.PICC_HaltA();
}
