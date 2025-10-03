# 🔬 Logicuino – Arduino-Based Logic Analyzer

Logicuino is a lightweight **logic analyzer** built on an **Arduino Uno / ATmega328P**.  
It samples up to **8 digital input channels (Pins D2–D9)** at a fixed rate using **Timer1**,  
stores them in a **circular buffer in RAM**, and streams the data via **UART** for visualization.  

---

## ✨ Features
- **8 input channels** (Pins 2–9 = Channels 1–8)  
- **Fixed sampling rate** using Timer1 interrupt  
- **Circular buffer** to handle continuous data acquisition  
- **High-speed UART streaming** (up to 1 Mbps)  
- Simple and efficient **single-byte data packing**  
  - Each bit of the byte corresponds to one digital channel state  
- Can be paired with a **Python viewer** for live waveform plotting (see below)  
- AVR-C is used to optimize parts of the code (faster sampling)

---

## ⚡ Hardware Requirements
- Arduino Uno (ATmega328P) or compatible board  
- USB cable to connect to host PC  
- Digital signal sources (0–5V logic) connected to pins **D2–D9**  

---

## 🛠 Arduino Configuration
- **Sample Rate**: configurable via `SAMPLE_RATE` (default: 80 kHz)  
- **Baud Rate**: configurable via `BAUD_RATE` (default: 1,000,000)  
- **Buffer Size**: power-of-two (default: 1024 bytes)  

### Pin Mapping
| Channel | Arduino Pin | ATmega Pin |
|---------|-------------|------------|
| CH1     | D2          | PD2        |
| CH2     | D3          | PD3        |
| CH3     | D4          | PD4        |
| CH4     | D5          | PD5        |
| CH5     | D6          | PD6        |
| CH6     | D7          | PD7        |
| CH7     | D8          | PB0        |
| CH8     | D9          | PB1        |

Each captured sample is a **single byte**, where:  
- **Bit0 → CH1 (D2)**  
- **Bit1 → CH2 (D3)**  
- …  
- **Bit7 → CH8 (D9)**  

---

## ▶️ Upload & Run
1. Open the `.ino` file in the Arduino IDE.  
2. Select **Board: Arduino Uno** and correct COM/Serial port.  
3. Upload the code.  
4. The Arduino will begin sampling immediately after reset.  
5. Use a serial terminal or a Python viewer to receive and interpret the stream.  

---

## 📊 Visualization
Pair this firmware with a Python-based viewer (example included in repo).  
The viewer can:  
- Plot each channel in its **own subplot**  
- Support **zoom, pan, and freeze**  
- Display signals in real-time, like a basic logic analyzer  

Viewer repo: [Logicuino Python Viewer](https://github.com/fduraibi/Logic-Analyzer_Viewer)  

---

## ⚠️ Notes & Limitations
- The **UART speed** and **sample rate** are tightly linked:  
  - Example: at 1 Mbps baud, ~80 kHz sample rate with 8 channels seems to be stable  
  - Higher sample rates may drop data if serial can’t keep up  
  - This is intended for hobbiest, results are not guarantered to be correct.
- Only **0–5V logic** is supported (direct to Arduino pins)  
- To handle higher voltages a special circuit is needed in the input side.
- For better timing accuracy, avoid using `delay()` or heavy code in `loop()`  

---

## 🚀 Future Work
- Allow some configurations to be done from the viewer side

---

## 📜 License
GPL3.  

---
