# Reflow Soldering DIY Controller

**Convert a standard consumer oven into a precise PCB reflow soldering station.**

This repository contains the complete electronics design, firmware, and mechanical files for a custom Reflow Oven Controller.The device is designed to precisely control a resistive heating element to follow the specific temperature profiles required for SMD soldering.

---

## 📂 Repository Structure

* **`Code/`**: Firmware source code for the STM32C0 microcontroller.
* **`Doc/`**: General project documentation and datasheets.
* **`Electronics/`**: Hardware design files, including the detailed technical rationale.
* **`3Dprint/`**: STL and CAD files for the controller enclosure.

---

## ⚡ System Overview

The controller operates as a closed-loop feedback system that regulates the temperature of a resistive heating element. 

### Key Technical Features
* **Power Control**: Designed for 220V / 50Hz mains up to **2400W** (12A design reference).
* **Switching Strategy**: Utilizes Zero-Crossing control to minimize EMI and power losses.
* **Galvanic Isolation**: Complete isolation between high-voltage AC and low-voltage logic via an Opto-TRIAC.
* **Microcontroller**: Powered by an **STM32C0 series** ARM Cortex-M0+.
* **User Interface**: Features an OLED display (128x64) and a 6-button navigation cluster.
* **Thermal Sensing**: Uses a glass-encapsulated NTC thermistor rated up to **300°C**.



---

## 🛠️ Hardware Specifications

The hardware design prioritizes low-cost, hand-solderable components and robust thermal management.

### Power Stage
| Component | Selection | Rationale |
| :--- | :--- | :--- |
| **TRIAC** | BTA16-600CW | 16A max current, snubberless design  |
| **Opto-TRIAC** | MOC3063M | Zero-crossing detection and safe isolation  |
| **Heatsink** | Boyd 530002B02500G | $2.6^{\circ}C/W$ resistance to handle ~13W dissipation  |

### Logic & Power
* **Input**: USB-C (5V).
* **Regulation**: L1117S33 LDO (SOT-223) provides a stable 3.3V rail for the MCU.
* **Optimization**: The temperature sensor uses an optimized $1.8k\Omega$ bridge resistor to maximize ADC voltage swing between $60^{\circ}C$ and $260^{\circ}C$.

---

## 🚀 Installation & Safety

### ⚠️ WARNING: High Voltage
This project involves **220V AC Mains**. Fatal injury or fire can occur if handled incorrectly. 
* Always ensure the device is unplugged before touching the electronics.
* Ensure the TRIAC heatsink is properly mounted as temperatures can reach $\approx 100^{\circ}C$ during operation.

### Setup
1. **Assembly**: Refer to the schematics in `Electronics/`. The design uses TSSOP-20 and SOT-223 packages for easy manual assembly.
2. **Firmware**: Flash the STM32C0 via the SWD interface pins (PA14/PA15).
3. **Calibration**: The system is tuned for a standard NTC thermistor (NRBG105F3950B1F).

---

## 👤 Author
**Valentin LEFEBVRE** (@topinambour)  
*Document Date: 06/02/2026*
