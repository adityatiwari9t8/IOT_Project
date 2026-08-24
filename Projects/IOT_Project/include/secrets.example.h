#pragma once
// =====================================================================
// secrets.example.h — TEMPLATE. Copy this file to "secrets.h" and fill
// in your real values. secrets.h is listed in .gitignore and will
// never be committed, so your Wi-Fi credentials and Google Script URL
// stay out of version control.
//
//   cp include/secrets.example.h include/secrets.h
//
// =====================================================================

// Wokwi's simulated network is always open and named "Wokwi-GUEST".
// Change these only if you flash this onto real hardware on a real network.
#define WIFI_SSID       "Wokwi-GUEST"
#define WIFI_PASSWORD   ""

// Your deployed Google Apps Script Web App URL (Deploy > New deployment
// > Web app > "Anyone" access). Ends in /exec.
#define GOOGLE_SCRIPT_URL "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec"

// A shared secret sent with every request so random strangers who guess
// your script URL can't write junk rows into your attendance sheet.
// This MUST match the API_KEY constant in the Apps Script backend
// (see backend/AppsScript.gs).
#define API_KEY "change-this-shared-secret"
