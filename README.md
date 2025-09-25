# My guide on Starting with ESP32

Hello, Bonjour, γεια σας  

This repo is made with the purpose of being a reference, a guide, for anyone starting with ESP32, specifically ESP32 with ESP-IDF, and who wants to follow the journey of a fellow beginner learner.  

---

## Disclaimer  

I’m not exactly a beginner in embedded software development, nor with ESP32.  

I’ve worked with Arduino, ESP32, and STM32 before. But my last internship was in a different engineering field (SCADA & HMIs for oil & gas). You could argue it’s not that different (they all come around to monitoring and control when boiled down), but my tasks there were pretty far from the embedded software I usually do (mostly Python). Hence, the motivation for this road trip with the ESP32.  

---

## Why ESP-IDF?  

Because I already worked with the Arduino framework before on an Arduino Uno (my first embedded software experience), and I also used PlatformIO on VS Code to program ESP32. I just wanted to try something tougher.  

So here we are. For this little road trip I bought an **ESP32-WROOM-32D board**, mind you, this is the first board I’ve ever actually owned. Before, I was always borrowing boards from companies I interned at, robotics clubs, or friends.  

This one has **dual-core, WiFi, Bluetooth, and a CP2104** so I can program it directly from my PC’s USB port.  

---

### Prep: Environment Setup  

- **Project:** Development environment setup
- **Tasks:** Install ESP-IDF & Test Flashing

->  [ESP-IDF Windows Setup Diary](https://github.com/TamaGo-HQ/espidf_get-started/blob/main/espidf-setup.md)
