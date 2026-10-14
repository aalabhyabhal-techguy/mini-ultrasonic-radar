
#include <Wire.h>
#include <U8g2lib.h>
#include <FastLED.h>
#include <Servo.h>
#include <math.h>

// ============== PIN CONFIGURATION ==============
#define TRIG_PIN       9
#define ECHO_PIN       10
#define LED_PIN        3
#define BUZZER_PIN     4
#define SERVO_PIN      6

#define NUM_LEDS       1
#define LED_BRIGHTNESS 45

// ============== RADAR SETTINGS ==============
#define MIN_ANGLE      20
#define MAX_ANGLE      160
#define ANGLE_STEP     9

#define MAX_RANGE_CM   200
#define DANGER_CM      10
#define WARNING_CM     30

// ============== TIMING ==============
#define SENSOR_INTERVAL_MS  60
#define DISPLAY_INTERVAL_MS 100
#define SERVO_INTERVAL_MS   25

#define WARNING_BEEP_MS     50
#define WARNING_PAUSE_MS    50

// ============== OLED ==============
U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(
  U8G2_R0, U8X8_PIN_NONE
);

// ============== DEVICES ==============
CRGB leds[NUM_LEDS];
Servo radarServo;

// ============== VARIABLES ==============
int scanAngle = MIN_ANGLE;
int scanDirection = 1;

float distanceCm = -1.0;

unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastServoUpdate = 0;

unsigned long buzzerTimer = 0;
bool warningBeepOn = false;

// ============== DISTANCE SENSOR ==============
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
    pulseIn(ECHO_PIN, HIGH, 12000UL);

  if (duration == 0) return -1.0;

  float cm = duration * 0.0343f / 2.0f;

  if (cm < 2.0f || cm > MAX_RANGE_CM) {
    return -1.0;
  }

  return cm;
}

// ============== FASTLED STATUS ==============
void updateLED(float d) {
  if (d < 0) {
    leds[0] = CRGB::Blue;
  }
  else if (d <= DANGER_CM) {
    leds[0] = CRGB::Red;
  }
  else if (d <= WARNING_CM) {
    leds[0] = CRGB(255, 80, 0);
  }
  else {
    leds[0] = CRGB::Green;
  }

  FastLED.show();
}

// ============== BUZZER LOGIC ==============
void updateBuzzer() {
  unsigned long now = millis();

  // DANGER: continuous sound
  if (distanceCm >= 0 && distanceCm <= DANGER_CM) {
    digitalWrite(BUZZER_PIN, HIGH);
    warningBeepOn = false;
    buzzerTimer = now;
    return;
  }

  // WARNING: 150 ms ON, 850 ms OFF
  if (distanceCm > DANGER_CM &&
      distanceCm <= WARNING_CM) {

    unsigned long elapsed = now - buzzerTimer;

    if (!warningBeepOn &&
        elapsed >= WARNING_PAUSE_MS) {
      warningBeepOn = true;
      buzzerTimer = now;
    }
    else if (warningBeepOn &&
             elapsed >= WARNING_BEEP_MS) {
      warningBeepOn = false;
      buzzerTimer = now;
    }

    digitalWrite(
      BUZZER_PIN,
      warningBeepOn ? HIGH : LOW
    );
    return;
  }

  // CLEAR or NO ECHO: silent
  digitalWrite(BUZZER_PIN, LOW);
  warningBeepOn = false;
  buzzerTimer = now;
}

// ============== ARC HELPER ==============
void drawArc(int cx, int cy, int radius,
             int startAngle, int endAngle) {
  float startRad = startAngle * PI / 180.0f;

  int prevX = cx + (int)(radius * cos(startRad));
  int prevY = cy - (int)(radius * sin(startRad));

  for (int a = startAngle + 3;
       a <= endAngle; a += 3) {
    float rad = a * PI / 180.0f;

    int x = cx + (int)(radius * cos(rad));
    int y = cy - (int)(radius * sin(rad));

    oled.drawLine(prevX, prevY, x, y);

    prevX = x;
    prevY = y;
  }

  // Ensure the exact endpoint is drawn
  float endRad = endAngle * PI / 180.0f;
  int endX = cx + (int)(radius * cos(endRad));
  int endY = cy - (int)(radius * sin(endRad));

  oled.drawLine(prevX, prevY, endX, endY);
}

// ============== RADAR OLED UI ==============
void drawDisplay() {
  const int cx = 64;
  const int cy = 62;
  const int radius = 34;

  oled.firstPage();

  do {
    oled.setFont(u8g2_font_5x7_tf);

    // Header
    oled.drawStr(0, 7, "MINI ULTRASONIC RADAR @FORGE");
    oled.drawHLine(0, 10, 128);

    // Distance display
    oled.setCursor(0, 20);
    oled.print("D:");

    if (distanceCm < 0) {
      oled.print(" NO ECHO");
    }
    else {
      oled.print(" ");
      oled.print(distanceCm, 1);
      oled.print("cm");
    }

    // Status label
    if (distanceCm < 0) {
      oled.drawStr(91, 20, "WAIT");
    }
    else if (distanceCm <= DANGER_CM) {
      oled.drawStr(91, 20, "DANGER");
    }
    else if (distanceCm <= WARNING_CM) {
      oled.drawStr(91, 20, "WARN");
    }
    else {
      oled.drawStr(91, 20, "CLEAR");
    }

    // Three range arcs, limited to the servo sweep
    drawArc(cx, cy, 12, MIN_ANGLE, MAX_ANGLE);
    drawArc(cx, cy, 23, MIN_ANGLE, MAX_ANGLE);
    drawArc(cx, cy, 34, MIN_ANGLE, MAX_ANGLE);

    // Range labels near the left side of the arc
    oled.drawStr(2, 43, "200");

    // Boundary lines
    float minRad = MIN_ANGLE * PI / 180.0f;
    float maxRad = MAX_ANGLE * PI / 180.0f;

    oled.drawLine(
      cx, cy,
      cx + (int)(radius * cos(minRad)),
      cy - (int)(radius * sin(minRad))
    );

    oled.drawLine(
      cx, cy,
      cx + (int)(radius * cos(maxRad)),
      cy - (int)(radius * sin(maxRad))
    );

    // Angle guides
    for (int a = 30; a < 180; a += 30) {
      if (a < MIN_ANGLE || a > MAX_ANGLE) continue;

      float rad = a * PI / 180.0f;

      int x = cx + (int)(radius * cos(rad));
      int y = cy - (int)(radius * sin(rad));

      oled.drawLine(cx, cy, x, y);
    }

    // Sweep line
    float rad = scanAngle * PI / 180.0f;

    int sx = cx + (int)(radius * cos(rad));
    int sy = cy - (int)(radius * sin(rad));

    oled.drawLine(cx, cy, sx, sy);

    // Distance marker
    if (distanceCm >= 0) {
      int markerRadius =
        (int)(distanceCm * radius / MAX_RANGE_CM);

      if (markerRadius < 2) markerRadius = 2;
      if (markerRadius > radius) markerRadius = radius;

      int mx = cx + (int)(markerRadius * cos(rad));
      int my = cy - (int)(markerRadius * sin(rad));

      oled.drawDisc(mx, my, 2);
    }

    // Centre point
    oled.drawDisc(cx, cy, 2);

    // Angle readout
    char angleText[12];
    snprintf(angleText, sizeof(angleText),
             "%03d deg", scanAngle);

    oled.drawStr(0, 62, angleText);

  } while (oled.nextPage());
}

// ============== SETUP ==============
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(
    leds, NUM_LEDS
  );

  FastLED.setBrightness(LED_BRIGHTNESS);
  leds[0] = CRGB::Blue;
  FastLED.show();

  oled.begin();

  radarServo.attach(SERVO_PIN);
  radarServo.write(scanAngle);

  buzzerTimer = millis();

  Serial.println("RADAR READY");
  Serial.println("Angle | Distance | Status");
}

// ============== MAIN LOOP ==============
void loop() {
  unsigned long now = millis();

  // Servo sweep
  if (now - lastServoUpdate >= SERVO_INTERVAL_MS) {
    lastServoUpdate = now;

    scanAngle += scanDirection * ANGLE_STEP;

    if (scanAngle >= MAX_ANGLE) {
      scanAngle = MAX_ANGLE;
      scanDirection = -1;
    }
    else if (scanAngle <= MIN_ANGLE) {
      scanAngle = MIN_ANGLE;
      scanDirection = 1;
    }

    radarServo.write(scanAngle);
  }

  // Sensor reading
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;

    distanceCm = readDistance();
    updateLED(distanceCm);

    Serial.print(scanAngle);
    Serial.print(" | ");

    if (distanceCm < 0) {
      Serial.print("NO ECHO");
    }
    else {
      Serial.print(distanceCm, 1);
      Serial.print(" cm");
    }

    Serial.print(" | ");

    if (distanceCm < 0) {
      Serial.println("NO ECHO");
    }
    else if (distanceCm <= DANGER_CM) {
      Serial.println("DANGER");
    }
    else if (distanceCm <= WARNING_CM) {
      Serial.println("WARNING");
    }
    else {
      Serial.println("CLEAR");
    }
  }

  // Buzzer is updated independently
  updateBuzzer();

  // OLED refresh
  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL_MS) {
    lastDisplayUpdate = now;
    drawDisplay();
  }
}
