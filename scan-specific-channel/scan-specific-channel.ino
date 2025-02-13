#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// CE- und CSN-Pins für das NRF24L01+ Modul
#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);
const uint8_t channel = 65;  // Festlegung des Kanals auf 60
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
  radio.setChannel(channel);                // Setze den Kanal auf 60
  radio.openReadingPipe(1, 0xE7E7E7E7E7LL); // Standardadresse einstellen
  radio.startListening();                   // Starte den Empfangsmodus
}
void loop() {
  if (radio.available()) {
    uint8_t receivedData[32];
    radio.read(&receivedData, sizeof(receivedData));

    Serial.print("📨 Daten empfangen auf Kanal ");
    Serial.print(channel);
    Serial.print(": ");
    for (int i = 0; i < sizeof(receivedData); i++) {
      Serial.print(receivedData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Interpretation der empfangenen Daten
    Serial.print("🔍 Interpretierte Daten (als ASCII): ");
    for (int i = 0; i < sizeof(receivedData); i++) {
      if (receivedData[i] >= 32 && receivedData[i] <= 126) {
        Serial.print((char)receivedData[i]);
      } else {
        Serial.print('.');
      }
    }
    Serial.println();
  }
  delay(100);
}
