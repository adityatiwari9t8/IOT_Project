#include "StudentDatabase.h"

StudentDatabase::StudentDatabase(Student* roster, int count)
    : _roster(roster), _count(count) {}

void StudentDatabase::begin() {
  // Open the "attendance" namespace in read/write mode (false).
  _prefs.begin("attendance", false);

  // Restore each student's saved state from a previous session. If a
  // UID was never written before, getBool()'s default (false) applies.
  for (int i = 0; i < _count; i++) {
    _roster[i].logged = _prefs.getBool(_roster[i].uid.c_str(), false);
  }
}

void StudentDatabase::resetAttendance() {
  _prefs.clear();  // wipes every key in the "attendance" namespace
  for (int i = 0; i < _count; i++) {
    _roster[i].logged = false;
  }
}

LookupResult StudentDatabase::lookup(const String& uid, int& outIndex) const {
  for (int i = 0; i < _count; i++) {
    if (_roster[i].uid == uid) {
      outIndex = i;  // valid either way — caller needs the name for both
      return _roster[i].logged ? LookupResult::ALREADY_LOGGED
                                : LookupResult::NEW_ENTRY;
    }
  }
  outIndex = -1;
  return LookupResult::UNKNOWN_CARD;
}

void StudentDatabase::markLogged(int index) {
  if (index >= 0 && index < _count) {
    _roster[index].logged = true;
    // Write through to flash immediately using the UID as the key.
    _prefs.putBool(_roster[index].uid.c_str(), true);
  }
}

const Student& StudentDatabase::at(int index) const {
  return _roster[index];
}

int StudentDatabase::count() const {
  return _count;
}
