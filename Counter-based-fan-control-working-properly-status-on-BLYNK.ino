// ESP32 Bidirectional Object Counter with LEDs/Fans + Relay + Blynk Cloud
// Ultrasonic A: TRIG = GPIO 19, ECHO = GPIO 18
// Ultrasonic B: TRIG = GPIO 5, ECHO = GPIO 4
// Built-in LED -> GPIO 2
// Relay -> GPIO 15 (manual control from Blynk V5)
// Counter Reset -> V6 button in Blynk

#define BLYNK_TEMPLATE_ID "TMPL3ZmCcbx7D"
#define BLYNK_TEMPLATE_NAME "CLASSROOM"
#define BLYNK_AUTH_TOKEN "jKjeYvBS9EF7tLnYzs8e4ohJzHBxcrDj"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// --- WiFi credentials ---
char ssid[] = "Mehraj Ali";        
char pass[] = "mehrajtheali";      

// --- Ultrasonic pins ---
#define TRIG_A 19
#define ECHO_A 18
#define TRIG_B 5
#define ECHO_B 4
#define LED_PIN 2   // Onboard ESP32 LED

// --- LED/Fan pins ---
#define LED1_PIN 23
#define LED2_PIN 22
#define LED3_PIN 21
#define RELAY_PIN 15   // Relay manual control from Blynk

long durationA, durationB;
float distanceA, distanceB;

int counter = 0;
unsigned long detectTime = 0;
bool A_first = false;
bool B_first = false;

BlynkTimer timer;

// --- Function to measure distance (cm) ---
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  float distance = (duration * 0.0343) / 2;
  if (distance == 0 || distance > 400) distance = 999; // ignore out of range
  return distance;
}

// --- Send LED status to Blynk ---
void sendBlynkData() {
  Blynk.virtualWrite(V4, digitalRead(LED_PIN));   // Onboard LED
  Blynk.virtualWrite(V1, digitalRead(LED1_PIN));  // LED1
  Blynk.virtualWrite(V2, digitalRead(LED2_PIN));  // LED2
  Blynk.virtualWrite(V3, digitalRead(LED3_PIN));  // LED3
}

// --- Blynk control for Relay only ---
BLYNK_WRITE(V5) {  
  int relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState);
  Serial.print("🔌 Relay State: ");
  Serial.println(relayState ? "ON" : "OFF");
}

// --- Blynk control to reset counter ---
BLYNK_WRITE(V7) {
  int resetBtn = param.asInt();
  if (resetBtn == 1) {   // Button pressed
    counter = 0;         // Reset counter
    Serial.println("🔄 Counter manually reset to 0!");
  }
}

void setup() {
  Serial.begin(115200);

  // Ultrasonic pins
  pinMode(TRIG_A, OUTPUT);
  pinMode(ECHO_A, INPUT);
  pinMode(TRIG_B, OUTPUT);
  pinMode(ECHO_B, INPUT);

  // LED/Fan pins
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);

  // Onboard LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // default OFF

  // --- Connect to WiFi ---
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);

  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 30) {
    delay(500);
    Serial.print(".");
    wifi_attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
    while (true) { delay(1000); }
  }

  // --- Initialize Blynk ---
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), pass);

  // Timer to send LED status every second
  timer.setInterval(1000L, sendBlynkData);

  Serial.println("Bidirectional Counter with Relay + Reset Ready...");
}

void loop() {
  Blynk.run();
  timer.run();

  distanceA = getDistance(TRIG_A, ECHO_A);
  distanceB = getDistance(TRIG_B, ECHO_B);

  // --- Detect A first ---
  if (!A_first && !B_first && distanceA <= 30) {
    A_first = true;
    detectTime = millis();
    Serial.println("Sensor A triggered first, waiting for B...");
  }

  // --- Detect B first ---
  if (!A_first && !B_first && distanceB <= 30) {
    B_first = true;
    detectTime = millis();
    Serial.println("Sensor B triggered first, waiting for A...");
  }

  // --- If A was first, check for B within 5s ---
  if (A_first) {
    if (distanceB <= 40 && millis() - detectTime <= 5000) {
      counter++;
      Serial.print("✅ Incremented, Count = ");
      Serial.println(counter);
      A_first = false;
    }
    if (millis() - detectTime > 3500) {
      Serial.println("⏳ Timeout, no increment.");
      A_first = false;
    }
  }

  // --- If B was first, check for A within 5s ---
  if (B_first) {
    if (distanceA <= 40 && millis() - detectTime <= 3500) {
      counter--;
      if (counter < 0) counter = 0; // avoid negative
      Serial.print("⬇️ Decremented, Count = ");
      Serial.println(counter);
      B_first = false;
    }
    if (millis() - detectTime > 3500) {
      Serial.println("⏳ Timeout, no decrement.");
      B_first = false;
    }
  }

  // --- LEDs controlled by counter ---
  digitalWrite(LED_PIN, (counter > 0) ? HIGH : LOW);        // Onboard LED
  digitalWrite(LED1_PIN, (counter > 10) ? HIGH : LOW);      // GPIO23
  digitalWrite(LED2_PIN, (counter > 20) ? HIGH : LOW);      // GPIO22
  digitalWrite(LED3_PIN, (counter > 30) ? HIGH : LOW);      // GPIO21

  delay(200);
}
