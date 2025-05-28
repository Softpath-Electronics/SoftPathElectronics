/* -----------------------------------------------------------------------------
   Beispiel-Sketch: Calibrate.ino
   Haftungsausschluss:
   Dieses Beispiel dient ausschließlich der Demonstration der SoftPath-Bibliothek.
   Es wurde nicht für produktive Anwendungen getestet. Vor jeder Nutzung müssen
   Hardware- und Software-Konfiguration auf Kompatibilität, Sicherheit und
   Normenkonformität geprüft werden. Verwendung auf eigenes Risiko!
   --------------------------------------------------------------------------- */

#include <SoftPathCalibrator.h>

SoftPathCalibrator calibrator;

void setup() {
  Serial.begin(115200);
  calibrator.begin();            // Startet den Kalibrier-Dialog
}

void loop() {
  /* kein Code notwendig */
}
