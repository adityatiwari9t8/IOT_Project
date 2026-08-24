#include "FeedbackIndicators.h"
#include <Arduino.h>
#include "config.h"

void FeedbackIndicators::begin() {
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_BLUE, LOW);
}

void FeedbackIndicators::success() {
  digitalWrite(PIN_LED_GREEN, HIGH);
  tone(PIN_BUZZER, TONE_SUCCESS_HZ, TONE_SUCCESS_MS);
  delay(SUCCESS_BEEP_HOLD_MS);
  digitalWrite(PIN_LED_GREEN, LOW);
}

void FeedbackIndicators::error() {
  digitalWrite(PIN_LED_RED, HIGH);
  tone(PIN_BUZZER, TONE_ERROR_HZ, TONE_ERROR_MS);
  delay(ERROR_BEEP_HOLD_MS);
  digitalWrite(PIN_LED_RED, LOW);
}

void FeedbackIndicators::wifiConnected() {
  tone(PIN_BUZZER, TONE_WIFI_OK_HZ, TONE_WIFI_OK_MS);
}

void FeedbackIndicators::networkBusy(bool on) {
  digitalWrite(PIN_LED_BLUE, on ? HIGH : LOW);
}
