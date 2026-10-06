#include <ESP8266WiFi.h>
#include <espnow.h>

typedef struct struct_pesan {
  int perintahId;
  int nilaiParameter;
} struct_pesan;

struct_pesan paketTerima;

void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  memcpy(&paketTerima, incomingData, sizeof(paketTerima));

  Serial.print("Paket masuk dari: ");
  Serial.println(macStr);
  Serial.print("Ukuran Data: ");
  Serial.print(len);
  Serial.println(" byte");
  Serial.print("Perintah ID: ");
  Serial.println(paketTerima.perintahId);
  Serial.print("Nilai Parameter: ");
  Serial.println(paketTerima.nilaiParameter);
  Serial.println("------------------------------------");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("Inisialisasi ESP-NOW Gagal!");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver ESP-NOW Menunggu Paket...");
}

void loop() {
  // Kosong
}