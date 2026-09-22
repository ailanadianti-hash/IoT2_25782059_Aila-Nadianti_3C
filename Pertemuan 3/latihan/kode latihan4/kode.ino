#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

const char *ssid = "samsung galaxy";
const char *password = "ailanadianti0";

ESP8266WebServer server(80);
const byte dhtPin = 2;       // Pin data DHT11 (GPIO 2 / D4)
const byte relayPin = 12;    // Pin kontrol LED/Relay (GPIO 12 / D6)
DHT dht(dhtPin, DHT11);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>IoT Dashboard - Latihan 2</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    .btn { padding: 15px 30px; font-size: 20px; border-radius: 8px; margin: 10px; cursor: pointer; text-decoration: none; display: inline-block; color: white; border: none; }
    .btn-on { background-color: #4CAF50; }
    .btn-off { background-color: #f44336; }
    .sensor-box { font-size: 24px; font-weight: bold; margin: 15px 0; }
  </style>
</head>
<body>
  <h1>SERVER Aila</h1>
  <div class="sensor-box">
    <p>Suhu Saat Ini: <strong>%TEMPERATURE%</strong> &deg;C</p>
    <p>Kelembapan: <strong>%HUMIDITY%</strong> %</p>
  </div>
  <h2>Kendali Relay</h2>
  %RELAY_BUTTON%
</body>
</html>
)rawliteral";

void handleRoot() {
  String html = index_html; 

  // Membaca suhu dan kelembapan dari sensor DHT
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Penggantian placeholder suhu
  if (isnan(t)) {
    html.replace("%TEMPERATURE%", "--");
  } else {
    html.replace("%TEMPERATURE%", String(t));
  }

  // 1. Penggantian placeholder kelembapan
  if (isnan(h)) {
    html.replace("%HUMIDITY%", "--");
  } else {
    html.replace("%HUMIDITY%", String(h));
  }

  // 2. Penggantian placeholder tombol dinamis tunggal (Toggle Button)
  String relayButtonHtml = "";
  if (digitalRead(relayPin) == HIGH) {
    // Jika LED sedang menyala, tampilkan tombol MATIKAN (OFF)
    relayButtonHtml = "<a href=\"/relay/off\"><button class=\"btn btn-off\">MATIKAN (OFF)</button></a>";
  } else {
    // Jika LED sedang mati, tampilkan tombol NYALAKAN (ON)
    relayButtonHtml = "<a href=\"/relay/on\"><button class=\"btn btn-on\">NYALAKAN (ON)</button></a>";
  }
  
  html.replace("%RELAY_BUTTON%", relayButtonHtml);
  
  server.send(200, "text/html", html);  
}

void handleRelayOn() { 
  digitalWrite(relayPin, HIGH); 
  server.sendHeader("Location", "/");   
  server.send(303);  
} 

void handleRelayOff() { 
  digitalWrite(relayPin, LOW);   
  server.sendHeader("Location", "/");   
  server.send(303);  
} 

void setup() {  
  Serial.begin(115200); 
  pinMode(relayPin, OUTPUT); 
  digitalWrite(relayPin, LOW);  
  dht.begin();  
    
  WiFi.mode(WIFI_STA);   
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }  
  Serial.println("\nIP Address Server Anda: ");  
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);  
  server.on("/relay/on", handleRelayOn);  
  server.on("/relay/off", handleRelayOff);  
  server.begin();  
} 

void loop() {  
  server.handleClient();  
}