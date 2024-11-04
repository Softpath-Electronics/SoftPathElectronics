/*
 * HAFTUNGSAUSSCHLUSS:
 * Diese Software wird "wie besehen" ohne ausdrückliche oder stillschweigende Garantie bereitgestellt.
 * In keinem Fall haftet der Autor oder das Unternehmen für Schäden, die durch die Verwendung dieser Software entstehen.
 * Es liegt in der Verantwortung des Nutzers sicherzustellen, dass die Software für seine Zwecke geeignet ist und dass
 * er sie vor der Verwendung ausreichend getestet hat.
 * Durch die Nutzung dieser Software erklären Sie, dass Sie die Sicherheitswarnungen und Gebrauchsanweisungen gelesen und verstanden haben.
 * 
 * DISCLAIMER:
 * This software is provided "as-is" without any express or implied warranty.
 * In no event shall the author or company be held liable for any damages arising from the use of this software.
 * It is the user's responsibility to ensure that the software is suitable for their needs and that they have tested it
 * sufficiently before using it in any project. By using this software, you agree that you have read and understood the safety warnings
 * and usage instructions.
 * 
 * LIZENZ:
 * Diese Software ist nur für den persönlichen, nicht-kommerziellen Gebrauch lizenziert. Eine kommerzielle Nutzung, 
 * Reproduktion oder Weiterverbreitung ist ohne vorherige schriftliche Zustimmung untersagt.
 * 
 * LICENSE:
 * This library is licensed for personal, non-commercial use only. Commercial use, reproduction, or redistribution without
 * prior written consent is prohibited.
 */

#include <SoftPathElectronics.h>

CustomKeyboard keyboard;

void setup() {
    Serial.begin(115200);
    // Entfernen der 'while (!Serial) { ; }' Zeile
    Serial.println("Keyboard Ready...");

    // Beispielschlüssel, ersetzen Sie diesen durch den Schlüssel, den Sie aus der Kalibrierung erhalten haben
    String key = "AUSGABE AUS DER KALIBRIERUNG HIER EINFÜGEN";
    keyboard.setupKey(key);
}

void loop() {
    int key = keyboard.getKeyPressed();
    if (key != -1) {
        Serial.print("Taste gedrückt: ");
        Serial.println(key);
    }
    delay(100); // Zum Entprellen
}
