#include "painlessMesh.h"
#include <ArduinoJson.h>

#define MESH_PREFIX     "SmartMeshNet"
#define MESH_PASSWORD   "passwordrahasia"
#define MESH_PORT       5555

#define LDR_PIN         A0     // Pin ADC A0 pada NodeMCU

Scheduler userScheduler;
painlessMesh mesh;

void sendMessage();
Task taskSendMessage(TASK_SECOND * 3, TASK_FOREVER, &sendMessage);

void sendMessage() {
  int adcValue = analogRead(LDR_PIN);

  // Kemas data ke dalam format JSON ringkas
  JsonDocument doc;
  doc["tipe"] = "cahaya_node";
  doc["adc"] = adcValue;

  String msgString;
  serializeJson(doc, msgString);

  mesh.sendBroadcast(msgString);
  Serial.println("Kirim Broadcast (LDR): " + msgString);
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