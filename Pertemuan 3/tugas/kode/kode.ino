#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

const char* ssid = "samsung galaxy";
const char* password = "ailanadianti1";

// Konfigurasi Pin
const byte dhtPin = 2;       // D4 (GPIO 2)
const byte buttonPin = 4;    // D2 (GPIO 4)
const byte ledTogglePin = 12;// D6 (GPIO 12) - LED Toggle Praktikum 1
const byte pwmLedPin = 5;    // D1 (GPIO 5)  - LED PWM Dimmer Tugas

DHT dht(dhtPin, DHT22);

// Variabel Pelacak Status
bool ledState = false;
String currentTemp = "--";
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
unsigned long lastTime = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ---------------- HTML & JAVASCRIPT (FRONT-END) ----------------
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Smart Room - PWM Dimmer</title>
  <style>
    body { font-family: Arial; text-align: center; }
    .card { background: #f0f0f0; margin: 20px auto; padding: 20px; max-width: 300px; border-radius: 10px; }
    button { padding: 15px 30px; font-size: 20px; border-radius: 5px; cursor: pointer; color: white;}
    .btn-on { background-color: #4CAF50; }
    .btn-off { background-color: #f44336; }
    input[type=range] { width: 100%; cursor: pointer; }
  </style>
</head>
<body>
  <h1>Smart Room Dashboard</h1>
  
  <div class="card">
    <h2>Suhu: <span id="tempValue">--</span> &deg;C</h2>
  </div>

  <div class="card">
    <h2>LED Toggle: <span id="ledStatus">OFF</span></h2>
    <button id="toggleBtn" class="btn-off" onclick="toggleLed()">Turn ON</button>
  </div>

  <div class="card">
    <h2>LED Dimmer (PWM)</h2>
    <input type="range" min="0" max="1023" id="pwmSlider" value="0" oninput="sendPWM(this.value)">
    <p>Nilai: <span id="pwmValue">0</span></p>
  </div>

  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;

    window.addEventListener('load', onLoad);
    function onLoad(event) { initWebSocket(); }
    function initWebSocket() {
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
    }
    function onOpen(event) { console.log('WebSocket Terkoneksi'); }
    function onClose(event) { setTimeout(initWebSocket, 2000); }

    function toggleLed(){ websocket.send('toggle'); }
    
    function sendPWM(value) {
      document.getElementById('pwmValue').innerHTML = value;
      websocket.send('pwm,' + value);
    }

    function onMessage(event) {
      var dataObj = JSON.parse(event.data);
      if(dataObj.suhu !== undefined) {
        document.getElementById('tempValue').innerHTML = dataObj.suhu;
      }
      if(dataObj.led !== undefined) {
        var btn = document.getElementById('toggleBtn');
        var status = document.getElementById('ledStatus');
        if(dataObj.led == "1"){
          status.innerHTML = "ON"; btn.innerHTML = "Turn OFF"; btn.className = "btn-on";
        } else {
          status.innerHTML = "OFF"; btn.innerHTML = "Turn ON"; btn.className = "btn-off";
        }
      }
    }
  </script>
</body>
</html>
)rawliteral";

// ---------------- BACK-END & WEBSOCKET LOGIC ----------------
void notifyClients() {
  String jsonString = "{\"led\":\"" + String(ledState ? 1 : 0) + "\",";
  jsonString += "\"suhu\":\"" + currentTemp + "\"}";
  ws.textAll(jsonString);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char*)data;

    // Deteksi perintah toggle lampu biasa
    if (message == "toggle") {
      ledState = !ledState;
      notifyClients();
    }
    // Deteksi string berformat "pwm,nilai" dari gesekan slider
    else if (message.startsWith("pwm,")) {
      int pwmValue = message.substring(4).toInt(); // Ambil angka setelah koma
      analogWrite(pwmLedPin, pwmValue);            // Ubah intensitas cahaya LED fisik
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Client WebSocket #%u terhubung\n", client->id());
      notifyClients();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Client WebSocket #%u terputus\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledTogglePin, OUTPUT);
  pinMode(pwmLedPin, OUTPUT);
  
  digitalWrite(ledTogglePin, LOW);
  analogWrite(pwmLedPin, 0); // Inisialisasi PWM mati
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP Address: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  ws.cleanupClients();
  
  // 1. Eksekusi perangkat keras LED Toggle
  digitalWrite(ledTogglePin, ledState ? HIGH : LOW);
  
  // 2. Baca Tombol Fisik (Debounce)
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        ledState = !ledState;
        notifyClients();
      }
    }
  }
  lastButtonState = reading;

  // 3. Baca DHT setiap 3 detik secara non-blocking
  if ((millis() - lastTime) > 3000) {
    float t = dht.readTemperature();
    if (!isnan(t)) {
      currentTemp = String(t);
      notifyClients();
    }
    lastTime = millis();
  }
}