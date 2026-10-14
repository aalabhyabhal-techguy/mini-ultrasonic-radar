#include <Wire.h>
#include <U8g2lib.h>
#include <FastLED.h>
#include <Servo.h>
#include <math.h>

// Arduino Nano radar system
// HC-SR04: TRIG D9, ECHO D10
// WS2812B LED: DIN D3
// Buzzer: D4
// Servo signal: D6
// SH1106 OLED: SDA A4, SCL A5

#define LED_PIN 3
#define NUM_LEDS 1
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
CRGB leds[NUM_LEDS];
Servo radarServo;

const byte TRIG_PIN = 9;
const byte ECHO_PIN = 10;
const byte BUZZER_PIN = 4;
const byte SERVO_PIN = 6;
const int SAFE_DISTANCE = 30;
const int DANGER_DISTANCE = 10;

int angle = 0;
int sweepDirection = 1;
unsigned long lastReadMs = 0;
float distanceCm = -1.0;

void setLed(const CRGB &color) {
  leds[0] = color;
  FastLED.show();
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) return -1.0;
  return duration * 0.0343f / 2.0f;
}

void drawRadar(float measuredDistance, int currentAngle) {
  const int cx = 64, cy = 56, radius = 36;
  u8g2.firstPage();
  do {
    u8g2.drawArc(cx, cy, 12, 180, 360);
    u8g2.drawArc(cx, cy, 24, 180, 360);
    u8g2.drawArc(cx, cy, radius, 180, 360);
    u8g2.drawHLine(cx - radius, cy, radius * 2);

    for (int a = 0; a <= 180; a += 30) {
      float rad = a * PI / 180.0f;
      u8g2.drawLine(cx, cy,
                    cx + (int)(radius * cos(rad)),
                    cy - (int)(radius * sin(rad)));
    }

    float rad = currentAngle * PI / 180.0f;
    int sx = cx + (int)(radius * cos(rad));
    int sy = cy - (int)(radius * sin(rad));
    u8g2.drawLine(cx, cy, sx, sy);

    if (measuredDistance >= 0 && measuredDistance <= 200) {
      int markerRadius = (int)(measuredDistance * radius / 200.0f);
      if (markerRadius < 2) markerRadius = 2;
      if (markerRadius > radius) markerRadius = radius;
      u8g2.drawDisc(cx + (int)(markerRadius * cos(rad)),
                    cy - (int)(markerRadius * sin(rad)), 2);
    }

    u8g2.setFont(u8g2_font_5x7_tf);
    char line[24];
    if (measuredDistance < 0) {
      u8g2.drawStr(0, 8, "RADAR: NO ECHO");
    } else {
      snprintf(line, sizeof(line), "DIST: %.1f cm", measuredDistance);
      u8g2.drawStr(0, 8, line);
    }

    if (measuredDistance >= 0 && measuredDistance <= DANGER_DISTANCE)
      u8g2.drawStr(88, 8, "DANGER");
    else if (measuredDistance >= 0 && measuredDistance <= SAFE_DISTANCE)
      u8g2.drawStr(88, 8, "WARN");
    else
      u8g2.drawStr(88, 8, "SAFE");
  } while (u8g2.nextPage());
}

void updateAlerts(float d) {
  if (d < 0) {
    setLed(CRGB::Blue);       // No echo
    noTone(BUZZER_PIN);
  } else if (d <= DANGER_DISTANCE) {
    setLed(CRGB::Red);        // Very close
    tone(BUZZER_PIN, 1800);
  } else if (d <= SAFE_DISTANCE) {
    setLed(CRGB::Orange);     // Caution
    tone(BUZZER_PIN, 900);
  } else {
    setLed(CRGB::Green);      // Clear
    noTone(BUZZER_PIN);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);

  // WS2812B LED setup using FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(40);
  setLed(CRGB::Blue);

  u8g2.begin();

  // Servo signal is D6. Power servo from a suitable external regulated 5V supply.
  radarServo.attach(SERVO_PIN);
  radarServo.write(angle);

  Serial.println(F("Arduino Radar System (FastLED) ready"));
  Serial.println(F("Angle,Distance_cm,Status"));
}

void loop() {
  radarServo.write(angle);

  unsigned long now = millis();
  if (now - lastReadMs >= 80) {
    lastReadMs = now;
    distanceCm = readDistanceCm();

    const char *status;
    if (distanceCm < 0) status = "NO_ECHO";
    else if (distanceCm <= DANGER_DISTANCE) status = "DANGER";
    else if (distanceCm <= SAFE_DISTANCE) status = "WARNING";
    else status = "SAFE";

    Serial.print(angle);
    Serial.print(',');
    if (distanceCm < 0) Serial.print(F("NO_ECHO"));
    else Serial.print(distanceCm, 1);
    Serial.print(',');
    Serial.println(status);

    updateAlerts(distanceCm);
    drawRadar(distanceCm, angle);
  }

  angle += sweepDirection * 3;
  if (angle >= 180) {
    angle = 180;
    sweepDirection = -1;
  } else if (angle <= 0) {
    angle = 0;
    sweepDirection = 1;
  }
  delay(25);
}
