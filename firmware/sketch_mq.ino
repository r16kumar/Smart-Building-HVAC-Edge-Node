#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include "DHT.h"

// --- Wi-Fi & ThingSpeak Credentials ---
const char* ssid = "YOUR_WIFI_NAME";  //as uploading on github, real name and password removed       
const char* password = "YOUR_WIFI_PASSWORD"; 
String apiKey = "YOUR_THINGSPEAK_API_KEY";          

// --- Pin Definitions & Constants ---
#define FAN_PIN 18
#define MQ135_PIN 34
#define DHTPIN 4
#define DHTTYPE DHT11

const int pwmFreq = 1000; 
const int pwmRes = 8; 

// --- OLED Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);
Adafruit_INA219 ina219;

// --- MULTITASKING TIMERS ---
unsigned long lastCloudUpdate = 0;
const unsigned long cloudInterval = 15000; // 15 seconds for ThingSpeak

void setup() {
  Serial.begin(115200);
  while (!Serial);

  ledcAttach(FAN_PIN, pwmFreq, pwmRes);
  ledcWrite(FAN_PIN, 0); 
  dht.begin();
  analogReadResolution(12);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  if(!ina219.begin()) { for(;;); }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Connecting Wi-Fi...");
  display.display();

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(0, 25); display.print("IP: "); display.println(WiFi.localIP());
  } else {
    display.setCursor(0, 25); display.println("Offline Mode Active.");
  }
  display.display();
  delay(2000);
}

void loop() {
  // --- 1. SENSE (Happens every 1 second now) ---
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  int rawADC = analogRead(MQ135_PIN);
  float pinVoltage = (rawADC * 3.3) / 4095.0;
  float mq135Voltage = pinVoltage * 1.5; 

  float power_mW = ina219.getPower_mW();

  // --- 2. THINK & ACT (Calibrated for your specific sensor) ---
  int fanSpeed = 0;
  int fanPercent = 0;

  if (mq135Voltage < 0.85) {
    fanSpeed = 0;     // Clean air (< 0.50V)
    fanPercent = 0;
  } 
  else if (mq135Voltage >= 0.85 && mq135Voltage < 0.90) {
    fanSpeed = 102;   // Light smoke (0.50V - 0.60V)
    fanPercent = 40;
  } 
  else if (mq135Voltage >= 0.90 && mq135Voltage < 1.10) {
    fanSpeed = 178;   // Moderate smoke (0.60V - 0.70V)
    fanPercent = 70;
  } 
  else {
    fanSpeed = 255;   // Heavy smoke (> 0.70V)
    fanPercent = 100;
  }
  ledcWrite(FAN_PIN, fanSpeed);

  // --- 3. LOCAL DASHBOARD (Updates every 1 second) ---
  display.clearDisplay();
  display.setCursor(0,0);  display.print("IAQ System - Live");
  display.setCursor(0, 15); display.print("Temp: "); display.print(temp, 1); display.print(" C");
  display.setCursor(64, 15); display.print("RH: "); display.print(hum, 1); display.print(" %");
  display.setCursor(0, 25); display.print("Gas: "); display.print(mq135Voltage, 2); display.print(" V");
  display.setCursor(0, 35); display.print("Fan Speed: "); display.print(fanPercent); display.print(" %");
  display.setCursor(0, 45); display.print("Power: "); display.print(power_mW, 1); display.print(" mW");
  display.display();

  // --- 4. CLOUD TRANSMISSION (Happens strictly every 15 seconds) ---
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastCloudUpdate >= cloudInterval) {
    lastCloudUpdate = currentMillis; // Reset the timer
    
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();            
      WiFi.begin(ssid, password);   
    } else {
      HTTPClient http;
      String url = "http://api.thingspeak.com/update?api_key=" + apiKey + 
                   "&field1=" + String(temp) + 
                   "&field2=" + String(hum) + 
                   "&field3=" + String(mq135Voltage) + 
                   "&field4=" + String(fanPercent) + 
                   "&field5=" + String(power_mW);
      
      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.print("ThingSpeak Update Success. HTTP Code: ");
        Serial.println(httpCode);
      }
      http.end(); 
    }
  }

  // A tiny 1-second delay so the screen doesn't flicker, and the fan reacts instantly
  delay(1000); 
}