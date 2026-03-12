#define BLYNK_TEMPLATE_ID "TMPL3q_IH67Da"
#define BLYNK_TEMPLATE_NAME "ROOM 1"
#define BLYNK_AUTH_TOKEN "aiB9Y3ooe-ZWfg24P1uIOjXGTavjJSV0"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Mehraj Ali";
char pass[] = "mehrajtheali22";

#define LED_PIN 2 // Built-in LED

// LED Control (V0)
BLYNK_WRITE(V0) {
  digitalWrite(LED_PIN, param.asInt());
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("LT21 (Room 1) is Online");
}

void loop() {
  Blynk.run();
  if (Serial.available() > 0) {
    int rollNo = Serial.parseInt();
    if (rollNo >= 1 && rollNo <= 10) {
      // LT21 ka data V1 par
      Blynk.virtualWrite(V1, "LT21 - Student Roll " + String(rollNo) + " Present\n");
      Serial.println("LT21: Roll " + String(rollNo));
    }
    while(Serial.available() > 0) Serial.read();
  }
}