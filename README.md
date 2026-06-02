# Energy-Aware HVAC Optimization Using Unsupervised Learning on Edge Hardware
### A Smart Building Edge AI Node

[![Watch the Hardware Demonstration](https://img.youtube.com/vi/z8VBz95kYA8/maxresdefault.jpg)](https://www.youtube.com/watch?v=z8VBz95kYA8)
*(Click the image above to watch the full hardware demonstration on YouTube)*

---

## 📌 Project Overview
This project demonstrates a predictive, energy-aware HVAC management system built on the **ESP32-WROOM-32** platform. It utilizes **TinyML (Edge AI)** to perform 100% offline anomaly detection on indoor air quality, specifically targeting Volatile Organic Compounds (VOCs).

Unlike traditional systems that use static "if-voltage > threshold" logic, this node employs an unsupervised **K-Means Clustering** algorithm. It processes a 60-second sliding window of air quality data to calculate a continuous anomaly distance metric. This allows for fine-grained, predictive fan speed control via **Pulse Width Modulation (PWM)**, maximizing energy efficiency by only drawing the power necessary for the detected hazard level.

**Developer:** [Rohit Kumar](https://github.com/your-github-profile)  
**Institution:** [Bharati Vidyapeeth's College of Engineering (BVCOE), New Delhi](https://main.bvcoend.ac.in/)  
**Course:** B.Tech Electronics & Communication Engineering (Final Year)  

---

## 🧠 TinyML Architecture
The machine learning pipeline was developed and trained using **Edge Impulse**, utilizing real-world data collected in New Delhi environments.

* **Algorithm:** K-Means Anomaly Detection (k=32 centroids).
* **Window Size:** 60,000 ms (total temporal context).
* **Stride Execution:** 15,000 ms (inference occurs every 15 seconds).
* **Feature Extraction:** Raw analog voltage from a hardware-calibrated MQ135 sensor.
* **Inference Speed:** ~2 ms (on device).

---

## ⚡ Energy-Aware Actuation Logic
The system translates the AI's "Anomaly Score" into precise fan speeds. The logic is optimized for a **12V 1A power budget**, with PWM duty cycles clamped at 94% to protect the MOSFET and motor electronics.

| Anomaly Score | Air Quality State | Fan Speed | Duty Cycle (8-bit) |
| :--- | :--- | :--- | :--- |
| **< 0.15** | Normal / Clean | **0% (OFF)** | 0 |
| **0.15 - 0.60** | Slight Anomaly | **40%** | 102 |
| **0.60 - 1.50** | Moderate Hazard | **70%** | 178 |
| **> 1.50** | Severe Hazard | **94%** | 240 |

---

## 🛠️ Hardware Specification
* **MCU:** ESP32-WROOM-32.
* **Gas Sensor:** MQ135 (Air Quality/VOCs).
* **Environmental Sensor:** DHT11 (Temperature & Humidity).
* **Power Monitor:** INA219 (Real time heater and system power logging).
* **Display:** SSD1306 128x64 I2C OLED.
* **Actuator:** 12V Brushless DC Fan controlled via IRLZ44N N-Channel MOSFET.
* **Power Rail:** 12V DC Adapter (1A) with LM2596 Buck Converter for 5V logic isolation.

---

## 🚀 Technical Highlights
1.  [cite_start]**Software-Defined Calibration:** Compensated for MQ135 "burn in" drift using a software multiplier (5.2x), enabling accurate 0.74V baselines through a physical voltage divider[cite: 1].
2.  [cite_start]**ADC-Wi-Fi Interference Fix:** Implemented an "Offline Wakeup" blip in the setup phase to initialize the ESP32's ADC power registers without requiring a constant 500mA Wi-Fi power draw[cite: 1].
3.  [cite_start]**Bootstrap Protection:** Clamped PWM output at 94% to ensure reliable operation of the MOSFET gate and fan motor capacitors during severe hazard events[cite: 1].

---

## 📂 Repository Structure
* `/firmware/` - Contains the `Smart_HVAC_Edge_Node.ino` sketch and K-Means library.
* `/docs/` - Circuit diagrams and data analysis summaries.
