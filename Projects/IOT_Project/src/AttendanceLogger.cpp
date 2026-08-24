#include "AttendanceLogger.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "config.h"
#include "secrets.h"

namespace {

// Percent-encodes a string for safe use in a URL query parameter.
// Student names are plain ASCII here, but this makes the code correct
// even if a name ever contains a space or symbol.
String urlEncode(const String& value) {
  String encoded;
  const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < value.length(); i++) {
    char c = value[i];
    if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ||
        c == '.' || c == '~') {
      encoded += c;
    } else {
      encoded += '%';
      encoded += hex[(c >> 4) & 0xF];
      encoded += hex[c & 0xF];
    }
  }
  return encoded;
}

}  // namespace

bool AttendanceLogger::connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(400);
    Serial.print(".");
  }

  _connected = (WiFi.status() == WL_CONNECTED);
  return _connected;
}

bool AttendanceLogger::isConnected() const {
  return _connected;
}

SyncResult AttendanceLogger::logAttendance(const String& uid,
                                            const String& name,
                                            int& outHttpCode) {
  outHttpCode = 0;

  if (!_connected || WiFi.status() != WL_CONNECTED) {
    return SyncResult::OFFLINE;
  }

  // Google's Apps Script endpoint is HTTPS. setInsecure() skips
  // certificate-chain validation, which is the standard approach for
  // ESP32 + Google Apps Script in student/hobby projects since Google
  // rotates its certs and the ESP32 has no easy way to keep a CA store
  // current. If you need real cert pinning for a production deployment,
  // load Google's root CA into WiFiClientSecure::setCACert() instead.
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(HTTP_REQUEST_TIMEOUT_MS / 1000);

  HTTPClient http;
  http.setConnectTimeout(HTTP_REQUEST_TIMEOUT_MS);
  http.setTimeout(HTTP_REQUEST_TIMEOUT_MS);

  String url = String(GOOGLE_SCRIPT_URL) + "?uid=" + urlEncode(uid) +
               "&name=" + urlEncode(name) + "&key=" + urlEncode(API_KEY);

  if (!http.begin(client, url)) {
    return SyncResult::HTTP_ERROR;
  }
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int code = http.GET();
  http.end();
  outHttpCode = code;

  if (code >= 200 && code < 300) {
    return SyncResult::SUCCESS;
  }
  return SyncResult::HTTP_ERROR;
}
