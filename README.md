# Smart Building Energy Management: Edge AI HVAC Node

## Overview
This repository contains the firmware, hardware architecture, and telemetry data for a custom IoT-based HVAC control node. Designed to replace wasteful binary (ON/OFF) ventilation systems, this edge node utilizes proportional PWM actuation driven by a local TinyML anomaly detection model.

By pushing compute capabilities directly to the edge, the system minimizes cloud latency and drastically reduces power consumption. Empirical validation over a 338-hour field deployment demonstrated a **98.61% energy reduction** compared to baseline systems.

---

## 1. Hardware Architecture & EMC
The hardware is built around an ESP32 dual-core microcontroller, interfacing with an analog MQ135 air quality sensor and driving an IRLZ44N MOSFET for high-current motor actuation. Power delivery is handled via an onboard LM2596 step-down converter.

**Electromagnetic Compatibility (EMC) Solutions:**
During initial prototyping, high switching noise from the motor driver induced severe Transient Voltage Sag and Ground Bounce, causing ADC sensor blindness. The final 2-layer PCB was engineered with a strict **Star Grounding topology**, physically and electrically isolating the noisy motor drive logic from the sensitive analog sensor paths.

* [View Custom PCB Schematic](hardware/HVAC_Node_Schematic.png)
* [View 3D Board Layout](hardware/PCB_3D_Layout.png)
* [View BOM & Pick/Place Manufacturing Files](hardware/)

---

## 2. Firmware & Edge AI (TinyML)
The firmware is written in bare-metal C++ utilizing the ESP32 Core 3.x API for direct hardware timer PWM generation. 

To eliminate reliance on cloud compute, an unsupervised **K-Means clustering algorithm** was trained via Edge Impulse and deployed natively onto the ESP32. The model evaluates raw ADC telemetry entirely offline within the strict 520 KB SRAM footprint, triggering proportional exhaust actuation only when a confirmed chemical anomaly breaches the threshold.

* [View Main Execution Firmware](firmware/hvac_edge_node_main.ino)
* [View MQ-135 ADC Calibration Utility](firmware/mq135_burn_in_calibration.ino)
* [View TinyML Anomaly Clusters](tinyml_models/Anomaly_Detection_Clusters.png)

---

## 3. Empirical Validation & Telemetry
System efficiency was validated using I2C power monitors (INA219) capturing live telemetry over 14 days. Data was parsed and integrated using Python (Pandas/Matplotlib).

**Results:**
* **Total Experiment Duration:** 338.0 Hours
* **Smart System Energy Used:** 5.17 Wh
* **Traditional Binary Fan Energy (Estimate):** 371.80 Wh
* **Total Energy Saved:** 98.61%

* [View Power Draw Telemetry Graph](telemetry_and_data/Energy_Savings_Validation.png)
* [View Dynamic IAQ vs. PWM Response Graph](telemetry_and_data/IAQ_PWM_Response.png)

---
*Architected and developed by Rohit Kumar.*
