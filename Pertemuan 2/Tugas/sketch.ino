#include "DHT.h"

const byte ldrPin = 34;       
const byte dhtPin = 14;       // Pindahkan ke pin 14 (standar DHT di Wokwi)
const byte relayPin = 12;     
const byte ledPin = 5;        

#define DHTTYPE DHT22 
DHT dht(dhtPin, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Pastikan aktuator mati saat awal
  digitalWrite(relayPin, LOW);
  digitalWrite(ledPin, LOW);
  
  Serial.println("Sistem Smart Warehouse Siap...");
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  float suhu = dht.readTemperature();

  if (isnan(suhu)) {
    Serial.println("Peringatan: Sensor DHT belum siap / Error!");
    delay(2000);
    return;
  }

  // Cetak pembacaan sensor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" C | LDR ADC: ");
  Serial.println(ldrValue);

  // Logika Rule Engine (OR Operator sesuai modul)
  if (suhu > 34.0 || ldrValue < 300) {
    digitalWrite(relayPin, HIGH); // Nyalakan Relay
    digitalWrite(ledPin, HIGH);   // Nyalakan LED
    Serial.println("-> Peringatan: Aktuator Aktif! (Kondisi Buruk)");
  } else {
    digitalWrite(relayPin, LOW);  // Matikan Relay
    digitalWrite(ledPin, LOW);    // Matikan LED
    Serial.println("-> Kondisi Aman");
  }

  delay(2000);
}