#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

#define RX_PIN 16  // ESP32 RX (Connect to TX of MPM12-BG)
#define TX_PIN 17  // ESP32 TX (Connect to RX of MPM12-BG)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C  // OLED I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HardwareSerial mySerial(1);

struct AQIBreakpoint {
    int C_low, C_high;
    int I_low, I_high;
};

// AQI Breakpoints for PM2.5 (EPA standard)
AQIBreakpoint pm25_aqi[] = {
    {0, 12, 0, 50}, {12, 35, 51, 100}, {35, 55, 101, 150}, {55, 150, 151, 200},
    {150, 250, 201, 300}, {250, 500, 301, 500}
};

// AQI Breakpoints for PM10 (EPA standard)
AQIBreakpoint pm10_aqi[] = {
    {0, 54, 0, 50}, {55, 154, 51, 100}, {155, 254, 101, 150}, {255, 354, 151, 200},
    {355, 424, 201, 300}, {425, 604, 301, 500}
};

int calculateAQI(int concentration, AQIBreakpoint breakpoints[], int size) {
    for (int i = 0; i < size; i++) {
        if (concentration >= breakpoints[i].C_low && concentration <= breakpoints[i].C_high) {
            return breakpoints[i].I_low + (concentration - breakpoints[i].C_low) *
                   (breakpoints[i].I_high - breakpoints[i].I_low) /
                   (breakpoints[i].C_high - breakpoints[i].C_low);
        }
    }
    return -1; // Invalid AQI
}

void drawArcReactorAnimation(int seconds) {
    for (int i = seconds; i > 0; i--) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(10, 5);
        display.print("Stabilizing: ");
        display.print(i);
        display.print("s");
        
        // Draw Arc Reactor-inspired animation lower on the screen
        display.drawCircle(64, 40, 20, WHITE); // Outer circle
        display.drawCircle(64, 40, 15, WHITE); // Inner circle
        display.drawCircle(64, 40, 5 + (i % 5), WHITE); // Pulsing center
        
        display.display();
        delay(1000);
    }
}

void setup() {
    Serial.begin(115200);
    mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("OLED initialization failed");
        for (;;);
    }
    display.clearDisplay();
    display.display();

    Serial.println("Waiting for sensor stabilization (30 sec)...");
    drawArcReactorAnimation(30);
    Serial.println("Sensor ready.");
    
}

void loop() {
    while (mySerial.available()) {
        mySerial.read();
    }

    unsigned long startTime = millis();
    while (mySerial.available() < 2) {
        if (millis() - startTime > 3000) { // Timeout after 3 seconds
            Serial.println("Error: Sensor data timeout");
            return;
        }
    }
    
    if (mySerial.read() != 0x42 || mySerial.read() != 0x4D) {
        Serial.println("Error: Invalid frame header");
        return;
    }

    uint8_t buffer[30];
    mySerial.readBytes(buffer, 30);

    int pm1_0 = buffer[8] << 8 | buffer[9];
    int pm2_5 = buffer[10] << 8 | buffer[11];
    int pm10  = buffer[12] << 8 | buffer[13];

    int aqi_pm25 = calculateAQI(pm2_5, pm25_aqi, 6);
    int aqi_pm10 = calculateAQI(pm10, pm10_aqi, 6);
    int final_aqi = max(aqi_pm25, aqi_pm10);

    Serial.print("PM1.0: "); Serial.print(pm1_0); Serial.print(" µg/m³, ");
    Serial.print("PM2.5: "); Serial.print(pm2_5); Serial.print(" µg/m³, ");
    Serial.print("PM10: "); Serial.print(pm10); Serial.print(" µg/m³");
    Serial.print("AQI (PM2.5): "); Serial.print(aqi_pm25); Serial.print(", ");
    Serial.print("AQI (PM10): "); Serial.print(aqi_pm10); Serial.print(", ");
    Serial.print("Final AQI: "); Serial.println(final_aqi);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("PM1.0: "); display.println(pm1_0);
    display.print("PM2.5: "); display.println(pm2_5);
    display.print("PM10: "); display.println(pm10);
    if (final_aqi == -1) {
        display.print("AQI: HAZARD");
    } else {
        display.print("AQI: "); display.println(final_aqi);
    }
    display.display();

    delay(1000);
}
