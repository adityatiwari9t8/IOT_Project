#pragma once
#include <Arduino.h>

enum class SyncResult {
  OFFLINE,       // Wi-Fi never connected — request wasn't attempted
  SUCCESS,       // HTTP 2xx from the Apps Script
  HTTP_ERROR     // request sent but got a non-2xx or connection error
};

// Owns the Wi-Fi connection and talks to the Google Apps Script backend
// over HTTPS. Everything network-related lives here so main.cpp only
// ever calls two methods: connect() once at boot, logAttendance() per tap.
class AttendanceLogger {
 public:
  // Attempts to join Wi-Fi, giving up after WIFI_CONNECT_TIMEOUT_MS.
  // Returns true if connected. On failure the device carries on in
  // offline mode instead of hanging forever.
  bool connect();

  bool isConnected() const;

  // Sends one attendance record. Blocks for up to HTTP_REQUEST_TIMEOUT_MS.
  // Returns HTTP_ERROR on any non-2xx response or transport failure —
  // the caller decides whether that should still count as "present".
  SyncResult logAttendance(const String& uid, const String& name,
                            int& outHttpCode);

 private:
  bool _connected = false;
};
