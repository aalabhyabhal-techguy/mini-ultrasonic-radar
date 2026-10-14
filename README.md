# mini-ultrasonic-radar
This is the firmware for a small radar built around the classic Arduino Nano, to make it "mini"

**Arduino Radar System**

A DIY radar project using an Arduino Nano, an HC-SR04 ultrasonic sensor, an SH1106 OLED display, an MG90S servo motor, a WS2812B LED, and a buzzer. It detects nearby objects, displays distance and radar graphics, and provides visual and sound alerts.

📡 **Arduino Radar System**

This project detects nearby objects and estimates their distance and direction using an ultrasonic sensor mounted on a rotating servo motor. An OLED displays radar graphics and distance readings, while an LED and buzzer provide visual and sound alerts to help identify nearby obstacles.

## Bill of Materials (BOM)

| S. No. | Component | Specification | Quantity |
|---:|---|---|---:|
| 1 | Arduino Nano | ATmega328P | 1 |
| 2 | Ultrasonic Sensor | HC-SR04 | 1 |
| 3 | OLED Display | 1.3-inch, 128 × 64, SH1106, I²C | 1 |
| 4 | Servo Motor | MG90S | 1 |
| 5 | RGB LED | WS2812B addressable LED | 1 |
| 6 | Active Buzzer | 5V active buzzer | 1 |
| 7 | Power Supply | Regulated 5V supply for servo | 1 |
| 8 | Breadboard | Solderless prototyping board | 1 |
| 9 | Jumper Wires | Compatible with Arduino connections | As required |
| 10 | USB Cable | Compatible with Arduino Nano | 1 |

### Software and Libraries

| Software / Library | Purpose |
|---|---|
| Arduino IDE | Writing and uploading the program |
| U8g2 | OLED display control |
| Servo | Servo motor control |
| FastLED | WS2812B LED control |

### Wiring Reference

- HC-SR04 TRIG → D9
- HC-SR04 ECHO → D10
- MG90S signal → D6
- WS2812B DIN → D3
- Active buzzer signal → D4
- OLED SDA → A4
- OLED SCL → A5

**Power note:** Power the servo using the external regulated 5V supply. Connect the external supply's ground to Arduino GND, but do not connect its positive output to the Nano's 5V rail while the Nano is powered by USB.

HERE ARE SOME IMAGES OF THE LENGTHY BUILD

FINAL PRODUCT✨
<img width="1200" height="1600" alt="WhatsApp Image 2026-10-14 at 17 53 02" src="https://github.com/user-attachments/assets/d4991838-7847-4207-8f0e-edc1ee2d2982" />

FACED POWER ISSUES HERE
<img width="899" height="1599" alt="IMG-20261010-WA0002" src="https://github.com/user-attachments/assets/de90bf1a-7125-47ed-8dac-8c36d0f341a5" />

OPENED UP
<img width="1917" height="976" alt="image" src="https://github.com/user-attachments/assets/7caf896c-be21-4493-855c-e89391fc8592" />


