#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <DHT.h>

#define MESH_PREFIX     "SmartMeshNet"
#define MESH_PASSWORD   "passwordrahasia"
#define MESH_PORT       5555

#define DHTPIN          2      // D4 (GPIO 2) pada NodeMCU
#define DHTTYPE         DHT11  // Ubah ke DHT22 jika menggunakan modul DHT22

DHT dht(DHTPIN, DHTTYPE);
Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();
Task taskSendMessage(TASK_SECOND * 3, TASK_FOREVER, &sendMessage);

void sendMessage() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca sensor DHT!");
    return;
  }

  // Kemas data ke dalam format JSON ringkas
  JsonDocument doc;
  doc["tipe"] = "suhu_node";
  doc["suhu"] = suhu;
  doc["kelembapan"] = kelembapan;

  String msgString;
  serializeJson(doc, msgString);

  mesh.sendBroadcast(msgString);
  Serial.println("Kirim Broadcast (DHT): " + msgString);
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Pesan dari %u: %s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("Koneksi baru, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.println("Topologi mesh berubah.");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  // Opsional
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  mesh.setDebugMsgTypes(ERROR | STARTUP);  
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}