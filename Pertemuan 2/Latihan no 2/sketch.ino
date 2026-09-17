const byte ldrPin = A0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  float persenCahaya = (ldrValue / 4095.0) * 100.0;

  Serial.print("Intensitas Cahaya (ADC): ");
  Serial.println(ldrValue);
  Serial.print("Persentase Cahaya: ");
  Serial.print(persenCahaya);
  Serial.println(" %");

  delay(1000);
}