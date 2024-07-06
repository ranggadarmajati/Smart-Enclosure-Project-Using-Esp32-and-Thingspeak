#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <ESP32Servo.h>

// Kredensial WiFi
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

// ThingSpeak
unsigned long myChannelNumber = your_channel;
const char* mywriteAPIKey = your_apikey;

// DHT Sensor
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Ultra Sonic Sensor
#define TRIGPIN 5
#define ECHOPIN 18

// Relay dan servo 
#define RELAYPIN 2 
#define SERVOPIN 13

WiFiClient client;
Servo servo;

float pos = 0.0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  pinMode(RELAYPIN, OUTPUT);
  servo.attach(SERVOPIN, 500, 2400);
  servo.write(pos);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan ke wifi...");
  }
  Serial.println("Terhubung ke wifi...");

  ThingSpeak.begin(client);
}

void loop() {
  // Read DHT sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Membaca Ultra Sonic sensor
  long duration, distance;
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);
  distance = (duration / 2) / 29.1;

  // Print Initial Sensor
  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.print(" °C, Kelembapan: ");
  Serial.print(humidity);
  Serial.print(" %, Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Implementasi Kontrol Otomatis
  if (temperature > 30.0) {
    // mengaktifkan relay jika suhu > 30 derajat celcius
    digitalWrite(RELAYPIN, HIGH);
    Serial.println("Relay Aktif");
    if (distance <= 200) {
      servo.write(90);
    } else {
      servo.write(pos);
    }
  } else {
    // Matikan relay jika suhu <= 30 derajat celcius
    digitalWrite(RELAYPIN, LOW);
    Serial.println("Relay Non Aktif");
    servo.write(pos);
  }

  // Tulis data ke ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, distance);

  int x = ThingSpeak.writeFields(myChannelNumber, mywriteAPIKey);

  if (x == 200) {
    Serial.println("Update Channel berhasil!");
  } else {
    Serial.println("Masalah update channel. Kode kesalahan HTTP " + String(x));
  }

  // waktu update data ke ThingSpeak 20 detik
  delay(20000);
}
