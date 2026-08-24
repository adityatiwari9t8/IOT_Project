/**
 * Smart Attendance System — Google Apps Script backend.
 *
 * Deploy: Extensions > Apps Script (from the target Sheet) > paste this
 * file in as Code.gs > Deploy > New deployment > Web app >
 *   Execute as: Me
 *   Who has access: Anyone
 * Copy the resulting /exec URL into include/secrets.h as
 * GOOGLE_SCRIPT_URL.
 *
 * Whenever you EDIT this file after the first deploy, you must also do
 * Deploy > Manage deployments > (pencil icon) > New version, otherwise
 * the live URL keeps serving the old code.
 */

// Must exactly match API_KEY in include/secrets.h on the firmware side.
// This is a simple shared-secret check — anyone who knows this string
// can write attendance rows, so treat it like a password and don't
// commit it anywhere public. It stops random bots that find the /exec
// URL from spamming your sheet; it is NOT strong cryptographic auth.
var API_KEY = 'lx9g6vTYQya58cqUvJdAtHUb';

var SHEET_HEADERS = ['Timestamp', 'Student Name', 'Card UID'];

function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  ensureHeaderRow_(sheet);

  var params = e.parameter || {};
  var key = params.key;
  var uid = params.uid;
  var name = params.name;

  if (key !== API_KEY) {
    return textResponse_('Error: Invalid or missing API key');
  }
  if (!uid || !name) {
    return textResponse_('Error: Missing uid or name parameter');
  }

  uid = uid.trim();
  name = name.trim();

  if (isDuplicateToday_(sheet, uid)) {
    // The ESP32 already tracks "already logged" in memory, but that
    // resets on every reboot — this server-side check is the backstop
    // that keeps the sheet clean even if the device restarts mid-class.
    return textResponse_(
      'Duplicate: ' + name + ' already logged today, no new row added'
    );
  }

  sheet.appendRow([new Date(), name, uid]);
  return textResponse_('Success: Attendance logged for ' + name);
}

function ensureHeaderRow_(sheet) {
  if (sheet.getLastRow() === 0) {
    sheet.appendRow(SHEET_HEADERS);
    sheet.getRange(1, 1, 1, SHEET_HEADERS.length).setFontWeight('bold');
  }
}

function isDuplicateToday_(sheet, uid) {
  var lastRow = sheet.getLastRow();
  if (lastRow < 2) return false;

  var todayStr = Utilities.formatDate(
    new Date(),
    Session.getScriptTimeZone(),
    'yyyy-MM-dd'
  );

  // Column A = Timestamp, Column C = Card UID (see SHEET_HEADERS above).
  var data = sheet.getRange(2, 1, lastRow - 1, 3).getValues();
  for (var i = 0; i < data.length; i++) {
    var rowDate = Utilities.formatDate(
      new Date(data[i][0]),
      Session.getScriptTimeZone(),
      'yyyy-MM-dd'
    );
    if (rowDate === todayStr && data[i][2] === uid) {
      return true;
    }
  }
  return false;
}

function textResponse_(message) {
  return ContentService.createTextOutput(message);
}
