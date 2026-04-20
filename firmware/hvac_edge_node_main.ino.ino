/*
 * Smart Building Energy Management System: Edge Node Firmware
 * Author: Rohit Kumar
 * Architecture: ESP32 (Xtensa LX6)
 * Description: Unsupervised K-Means clustering algorithm deployed via TinyML, optimized to execute entirely offline within the ESP32’s strict 520 KB SRAM footprint.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include "DHT.h"

// --- Hardware Pin Definitions ---
#define MQ135_PIN       34  // Analog pin for Chemical Sensing
#define DHT_PIN         4   // Digital pin for Psychrometric telemetry
#define MOSFET_PIN      25  // PWM capable pin for IRLZ44N Gate

// --- Display Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Object Instantiation ---
Adafruit_INA219 ina219;
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// --- ESP32 PWM Configuration for MOSFET ---
const int pwmFreq = 5000;
const int pwmResolution = 8; // 0-255 scale

// --- System Variables ---
float mq135_voltage = 0.0;
float current_mA = 0.0;
float anomaly_score = 0.0; 
const float ANOMALY_THRESHOLD = 0.80; // Your defined TinyML trigger point

void setup() {
  Serial.begin(115200);
  
  // =========================================================================
  // CRITICAL FIX: The Boot-Up Race Condition Mitigation
  // Gives the AMS1117 3.3V rail and OLED internal RAM time to stabilize 
  // before the ESP32 blasts I2C initialization commands.
  // =========================================================================
  delay(500); 

  // Force a stable, slow I2C bus
  Wire.begin(); 
  Wire.setClock(100000); // 100 kHz Standard Mode 

  // 1. Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Force a hard clear immediately
  display.clearDisplay();
  display.display(); 
  display.setTextColor(SSD1306_WHITE);

  // 2. Initialize INA219 Power Profiler
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
  }

  // 3. Initialize DHT11
  dht.begin();

  // 4. Configure MOSFET PWM Channel (ESP32 Core 3.x API)
  ledcAttach(MOSFET_PIN, pwmFreq, pwmResolution);
  ledcWrite(MOSFET_PIN, 0); // Ensure fan is OFF at boot

  // Print Boot Screen
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.println("System Booting...");
  display.println("Warming MQ135...");
  display.display();
  delay(2000); // Allow sensor brief warmup before loop starts
}

void loop() {
  // --- A. Data Acquisition ---
  
  // Read Analog Chemical Data (12-bit ADC: 0-4095)
  int raw_adc = analogRead(MQ135_PIN);
  mq135_voltage = (raw_adc / 4095.0) * 3.3;

  // Read Psychrometric Data (For telemetry, isolated from ML)
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Read Power Profiling
  current_mA = ina219.getCurrent_mA();


  // --- B. The Edge AI Drop-Zone ---
  
  /* * [TINYML INTEGRATION POINT]
   * Here is where you will call your Edge Impulse sliding window function.
   * Example: 
   * run_kmeans_inference(mq135_voltage, &anomaly_score);
   */
   
  // For testing the physical hardware before the AI is active, we will 
  // artificially link the anomaly score directly to the raw voltage.
  anomaly_score = mq135_voltage; 


  // --- C. MOSFET Actuation Logic ---
  
  if (anomaly_score >= ANOMALY_THRESHOLD) {
    // Danger detected: Ramp up the exhaust fan
    Serial.println("THRESHOLD BREACHED: Actuating HVAC Load!");
    ledcWrite(MOSFET_PIN, 255); // Write directly to the pin
  } else {
    // Air is clean: Turn off exhaust to save energy
    ledcWrite(MOSFET_PIN, 0);   // Write directly to the pin
  }


  // --- D. OLED User Interface ---
  
  display.clearDisplay();
  
  // Row 1: Chemical Data & AI Score
  display.setCursor(0, 0);
  display.print("Volts: "); display.print(mq135_voltage, 2); display.println(" V");
  display.print("Score: "); display.print(anomaly_score, 2);
  
  // Row 2: Status Indicator
  display.setCursor(80, 8);
  if(anomaly_score >= ANOMALY_THRESHOLD) {
    display.print("[FAN ON]");
  } else {
    display.print("[SAFE]");
  }

  // Row 3: Environmental Data
  display.setCursor(0, 24);
  display.print("T: "); display.print(temp, 1); display.print("C  ");
  display.print("H: "); display.print(hum, 0); display.println("%");

  // Row 4: Power Profiling
  display.setCursor(0, 40);
  display.print("Draw: "); display.print(current_mA, 1); display.println(" mA");

  display.display();

  // Evaluate the sliding window loop at standard RTOS intervals
  // (Set to 1000ms for active debugging, change to 15000ms for final deployment)
  delay(1000); 
}