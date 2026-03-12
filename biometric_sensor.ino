#define BLYNK_TEMPLATE_ID "TMPL3Ff2uS15h"
#define BLYNK_TEMPLATE_NAME "BIOMETRIC ROOM"
#define BLYNK_AUTH_TOKEN "8OB-mmuzJ04kAyZN1YjCyf6sVZZx5Gfw"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Fingerprint.h>

char ssid[] = "Mehraj Ali";
char pass[] = "mehrajtheali22";

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

BlynkTimer timer;

// 5 Students (Finger ID 1–5)
String names[6] = {"", "Aarav", "Rohit", "Neha", "Suman", "Imran"};
bool isInside[6] = {false, false, false, false, false, false};

int insideCount = 0;

unsigned long lastScanTime = 0;
int lastID = 0;

// ======================= WiFi + Blynk Reconnect =======================
void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WiFi...");
    WiFi.begin(ssid, pass);
  }

  if (!Blynk.connected()) {
    Serial.println("Reconnecting Blynk...");
    Blynk.connect();
  }
}

// ======================= Setup =======================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("System Starting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected ✅");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  if (Blynk.connected())
    Serial.println("Blynk Connected ✅");
  else
    Serial.println("Blynk Failed ❌");

  // Fingerprint Setup
  mySerial.begin(57600, SERIAL_8N1, 4, 5);
  finger.begin(57600);

  if (finger.verifyPassword())
    Serial.println("Fingerprint Sensor Ready ✅");
  else
    Serial.println("Fingerprint Sensor Error ❌");

  timer.setInterval(5000L, checkConnection);
}

// ======================= Loop =======================
void loop() {
  Blynk.run();
  timer.run();
  checkFingerprint();
}

// ======================= Fingerprint Logic =======================
void checkFingerprint() {

  if (finger.getImage() != FINGERPRINT_OK) return;
  if (finger.image2Tz() != FINGERPRINT_OK) return;
  if (finger.fingerFastSearch() != FINGERPRINT_OK) {
    Serial.println("Unknown Finger ❌");
    return;
  }

  int id = finger.fingerID;

  if (id < 1 || id > 5) return;

  // Prevent double scan within 3 sec
  if (id == lastID && millis() - lastScanTime < 3000) {
    return;
  }

  lastID = id;
  lastScanTime = millis();

  if (!isInside[id]) {

    isInside[id] = true;
    insideCount++;

    Serial.println(names[id] + " -> IN");
    Blynk.virtualWrite(V0, names[id] + " -> IN");

  } else {

    isInside[id] = false;

    if (insideCount > 0) insideCount--;

    Serial.println(names[id] + " -> OUT");
    Blynk.virtualWrite(V0, names[id] + " -> OUT");
  }

  Serial.print("Total Inside: ");
  Serial.println(insideCount);

  Blynk.virtualWrite(V1, insideCount);
}