/* -----------------------------------------------------------------------------
   Beispiel-Sketch: UseKeyboard.ino
   Haftungsausschluss:
   Dieser Code ist ein einfaches Demonstrationsbeispiel zum Einlesen eines
   analogen Keypads mit SoftPath. Prüfen Sie vor dem Einsatz, ob die Bibliothek
   und dieses Beispiel zur eingesetzten Hardware-/Software-Umgebung passen.
   Der Autor übernimmt keinerlei Haftung für Schäden oder Fehlfunktionen.
   --------------------------------------------------------------------------- */

#include <SoftPathElectronics.h>

/* Ersetzen Sie den folgenden Platzhalter-String durch den von der Kalibrierung
   erzeugten Key-String. */
const char KEY[] =
  "SPK1 0 2 6 12 1 1023 318 187 510 241 157";   // <- Platzhalter

SoftPathElectronics keypad;

void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  if (!keypad.loadKey(KEY)) {
    Serial.println(F("Ungültiger Key-String!"));
    while (true) { }
  }

  keypad.setDebug(false);                     // Debug-Ausgaben
  keypad.begin();
}

void loop() {
  // Liest den aktuellen Tastenzustand vom Keypad aus
  uint8_t k = keypad.read();

  // Wenn eine Taste erkannt wurde (k != 0), wird sie ausgegeben
  if (k) {
    // Ausgabe der erkannten Taste über den seriellen Monitor
    Serial.print(F("Gedrückt: Taste "));
    Serial.println(k);  // Gibt die Nummer der gedrückten Taste aus
  }

  // Hinweis: Wird keine Taste gedrückt, liefert read() den Wert 0,
  // und es erfolgt keine Ausgabe. So wird unnötige Ausgabe vermieden.
}
