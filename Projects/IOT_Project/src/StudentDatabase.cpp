#include "StudentDatabase.h"

StudentDatabase::StudentDatabase(Student* roster, int count)
    : _roster(roster), _count(count) {}

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
  }
}

const Student& StudentDatabase::at(int index) const {
  return _roster[index];
}

int StudentDatabase::count() const {
  return _count;
}
