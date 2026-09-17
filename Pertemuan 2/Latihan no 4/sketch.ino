const byte ldrPin = A0;
const byte ledPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  int ldrValue = analogRead(ldrPin);
  float persenCahaya = (ldrValue / 4095.0) * 100.0;

  if (ldrValue < 800) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Gelap! LED menyala.");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("Cukup terang. LED mati.");
  }

  Serial.print("Intensitas Cahaya (ADC): ");
  Serial.println(ldrValue);
  Serial.print("Persentase Cahaya: ");
  Serial.print(persenCahaya);
  Serial.println(" %");

  delay(1000);
}