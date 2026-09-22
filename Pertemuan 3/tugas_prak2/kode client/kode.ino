#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>

const char *ssid = "samsung galaxy";
const char *password = "ailanadianti0";
const char *serverName = "http://192.168.1.15/relay/on"; // Ganti dengan IP Server teman Anda

const byte dhtPin = 2; // Pin data DHT11 (GPIO 2 / D4)
DHT dht(dhtPin, DHT11);

void setup() {  
  Serial.begin(115200);  
  dht.begin();
  
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }  
  Serial.println("\nClient Terhubung ke Wi-Fi!");  
} 

void loop() {  
  // Membaca suhu dari sensor DHT (menggantikan ldrValue dari Praktikum 2)
  float t = dht.readTemperature();

  // Cek apakah pembacaan sensor valid
  if (isnan(t)) {
    Serial.println("Gagal membaca sensor DHT!");
    delay(2000);
    return;
  }

  Serial.print("Suhu saat ini: ");
  Serial.print(t);
  Serial.println(" C");

  // Perintah Latihan 4: Kirim HTTP GET HANYA JIKA Wi-Fi terhubung DAN suhu > 35
  if ((WiFi.status() == WL_CONNECTED) && (t > 35.0)) {  
    WiFiClient client;  
    HTTPClient http;  
      
    http.begin(client, serverName); 
    int httpResponseCode = http.GET();  
      
    Serial.print("Suhu > 35C! HTTP Response code: ");  
    Serial.println(httpResponseCode); 
      
    http.end(); 
    
    // Cooldown agar tidak spam request ke server
    delay(10000); 
  } 
  
  delay(2000);  
}