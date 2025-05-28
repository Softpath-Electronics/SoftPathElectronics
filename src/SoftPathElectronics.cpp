// SoftPathElectronics.cpp
#include "SoftPathElectronics.h"

SoftPathElectronics::SoftPathElectronics() :
  _pin(0), _numKeys(0), _tol(0),
  _debounceAfterZero(false), _debug(false), _waitingZero(false)
{ }

/* ---------- Key-String laden ---------- */

bool SoftPathElectronics::loadKey(const char* key) {
  char buf[192];
  size_t len = strlen(key);
  if (len >= sizeof(buf)) return false;
  memcpy(buf, key, len + 1);

  char* save;
  char* tok = strtok_r(buf, " ", &save);
  if (!tok || strcmp(tok, "SPK1")) return false;

  tok = strtok_r(NULL, " ", &save); if (!tok) return false;
  uint8_t keyPlatform = (uint8_t)atoi(tok);

  tok = strtok_r(NULL, " ", &save); if (!tok) return false;
  _pin = (uint8_t)atoi(tok);

  tok = strtok_r(NULL, " ", &save); if (!tok) return false;
  _numKeys = (uint8_t)atoi(tok);
  if (_numKeys == 0 || _numKeys > 16) return false;

  tok = strtok_r(NULL, " ", &save); if (!tok) return false;
  _tol = (uint16_t)atoi(tok);

  tok = strtok_r(NULL, " ", &save); if (!tok) return false;
  _debounceAfterZero = atoi(tok) ? true : false;

  for (uint8_t i = 0; i < _numKeys; ++i) {
    tok = strtok_r(NULL, " \r\n", &save);
    if (!tok) return false;
    _values[i] = (uint16_t)atoi(tok);
  }

  if (keyPlatform != SPK_PLATFORM) {
    if (keyPlatform == 0 && SPK_PLATFORM == 1) {
      for (uint8_t i = 0; i < _numKeys; ++i) _values[i] <<= 2;
      _tol <<= 2;
    } else if (keyPlatform == 1 && SPK_PLATFORM == 0) {
      for (uint8_t i = 0; i < _numKeys; ++i) _values[i] >>= 2;
      _tol >>= 2;
    }
  }
  _waitingZero = false;
  return true;
}

/* ---------- Initialisierung ---------- */

void SoftPathElectronics::begin() {
#if SPK_PLATFORM == 0
  pinMode(_pin + A0, INPUT);
#else
  pinMode(_pin, INPUT);
#endif
}

uint16_t SoftPathElectronics::analogReadNow() const {
#if SPK_PLATFORM == 0
  return analogRead(_pin + A0);
#else
  return analogRead(_pin);
#endif
}

/* ---------- Lesen ---------- */

uint8_t SoftPathElectronics::read() {
  uint16_t raw = analogReadNow();
  if (_debug) { Serial.print(F("ADC=")); Serial.println(raw); }

  if (_debounceAfterZero) {
    if (_waitingZero) {
      if (raw < _tol) _waitingZero = false;
      return 0;
    }
  }

  for (uint8_t i = 0; i < _numKeys; ++i) {
    uint16_t tgt = _values[i];
    if (raw >= tgt - _tol && raw <= tgt + _tol) {
      if (_debug) { Serial.print(F("Taste ")); Serial.println(i + 1); }
      if (_debounceAfterZero) _waitingZero = true;
      return i + 1;
    }
  }
  return 0;
}
