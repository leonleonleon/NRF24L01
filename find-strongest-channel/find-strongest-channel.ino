#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// CE- und CSN-Pins für das NRF24L01-Modul
#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);

// Anzahl der Messungen pro Kanal zur genauen Mittelung der Signalstärke
const int measurements = 500;  // Anzahl der Messungen pro Kanal
const int scans = 10;          // Anzahl der Scandurchläufe

// Variable zum Speichern der Signalstärke für jeden Kanal über alle Scans hinweg
float channelSignalStrength[126];      // Akkumulierte Signalstärke pro Kanal
float averageSignalStrength[126];      // Durchschnittliche Signalstärke pro Kanal
void setup() {
  Serial.begin(115200);
  while (!Serial);  // Warte, bis der serielle Monitor verbunden ist

  if (!radio.begin()) {
    Serial.println("✖️ NRF24L01-Modul nicht gefunden. Bitte Verkabelung überprüfen.");
    while (1);  // Stoppe das Programm, falls das Modul nicht gefunden wird
  } else {
    Serial.println("✅ NRF24L01-Modul erfolgreich initialisiert.");
  }

  // Einstellungen des Funkmoduls
  radio.setAutoAck(false);                  // Automatische Bestätigungen deaktivieren
  radio.disableCRC();                       // CRC-Prüfung deaktivieren
  radio.setDataRate(RF24_250KBPS);          // Datenrate auf 250 Kbps setzen für höhere Empfindlichkeit
  radio.setPALevel(RF24_PA_MAX);            // Maximale Leistung
  radio.openReadingPipe(1, 0xE7E7E7E7E7LL); // Standardadresse einstellen

  // Initialisiere die Arrays
  for (int i = 0; i < 126; i++) {
    channelSignalStrength[i] = 0;
    averageSignalStrength[i] = 0;
  }
}
void loop() {
  Serial.println("🔄 Starte mehrfachen Kanalscan...");

  for (int scanNumber = 1; scanNumber <= scans; scanNumber++) {
    Serial.print("🔎 Kanalscan Durchlauf ");
    Serial.print(scanNumber);
    Serial.println(" von 10");

    // Durchlaufe alle Kanäle von 0 bis 125
    for (uint8_t channel = 0; channel <= 125; channel++) {
      radio.setChannel(channel);
      radio.startListening();

      int rpdCount = 0;

      // Mehrfache Messungen auf dem aktuellen Kanal
      for (int i = 0; i < measurements; i++) {
        if (radio.testRPD()) {
          rpdCount++;
        }
        delayMicroseconds(500);  // Wartezeit zwischen den Messungen
      }

      radio.stopListening();

      // Berechne den Prozentsatz der positiven RPD-Messungen für diesen Scan
      float rpdPercentage = (rpdCount / (float)measurements) * 100.0;
      
      // Akkumuliere die Signalstärke über alle Scans hinweg
      channelSignalStrength[channel] += rpdPercentage;

      // Fortschrittsanzeige
      if (channel % 25 == 0) {  // Alle 25 Kanäle eine Fortschrittsmeldung
        Serial.print("➡️ Durchlauf ");
        Serial.print(scanNumber);
        Serial.print(": Fortschritt ");
        Serial.print(((channel + 1) * 100) / 126);
        Serial.println("%");
      }
    }

    Serial.println();
  }

  // Berechne die durchschnittliche Signalstärke pro Kanal
  for (int i = 0; i < 126; i++) {
    averageSignalStrength[i] = channelSignalStrength[i] / scans;
  }

  // Suche den Kanal mit der maximalen durchschnittlichen Signalstärke
  float maxSignal = 0;
  int bestChannel = -1;
  for (int i = 0; i < 126; i++) {
    if (averageSignalStrength[i] > maxSignal) {
      maxSignal = averageSignalStrength[i];
      bestChannel = i;
    }
  }

  // Ausgabe der Ergebnisse
  if (bestChannel != -1) {
    Serial.println("📈 Stärkster Kanal über alle Scans hinweg:");
    Serial.print("🔸 Kanal: ");
    Serial.println(bestChannel);
    Serial.print("🔸 Durchschnittliche Signalstärke: ");
    Serial.print(maxSignal, 2);
    Serial.println("%\n");
  } else {
    Serial.println("❌ Kein Signal auf den Kanälen 0-125 erkannt.\n");
  }

  // Optionale detaillierte Ausgabe der Signalstärke pro Kanal
  Serial.println("📊 Durchschnittliche Signalstärke pro Kanal:");
  for (int i = 0; i < 126; i++) {
    if (averageSignalStrength[i] > 0) {
      Serial.print("Kanal ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(averageSignalStrength[i], 2);
      Serial.println("%");
    }
  }

  // Beende das Programm oder warte vor erneutem Scan
  Serial.println("\n🏁 Mehrfacher Kanalscan abgeschlossen.");
  
  while (1);  // Programm anhalten
}
