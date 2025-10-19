# ESP32-CAM MJPEG Streaming Example

A simple project to stream MJPEG video from an **AI-Thinker ESP32-CAM** over Wi-Fi using the ESP-IDF framework.  
This repository is meant as a **starter guide for beginners** to explore camera streaming with ESP32.

## Features

- Wi-Fi Station (STA) mode: connects ESP32-CAM to your home Wi-Fi.  
- MJPEG streaming over HTTP: accessible in a browser via `http://<ESP_IP>/stream`.  
- Adjustable camera parameters: frame size and JPEG quality.  
- Easy-to-understand ESP-IDF setup with Wi-Fi events and HTTP server.

## Hardware Required

- AI-Thinker ESP32-CAM board  
- USB-to-Serial adapter for flashing  
- Small fixture to hold the camera (optional)  
- Pink yarn, tape, or soldering fixator to stabilize the camera

##  Camera Setup

- The ESP32-CAM was mounted using a soldering fixator and yarn for stability.  
- Lighting was constant during testing.  
- Frame size: `FRAMESIZE_SVGA`  
- JPEG quality: `20` (lower values = better quality, slower encoding)

## Software

- **ESP-IDF**: tested with the latest stable release.  
- **esp32-camera driver**: from [Espressif GitHub repo](https://github.com/espressif/esp32-camera).  
- Dependencies are declared in `CMakeLists.txt`:

## How It Works

1. Wi-Fi Initialization
- STA mode connects the ESP32 to your router.  
- Handles events like disconnects and IP acquisition.  

2. Camera Initialization
- Sets up the camera using `camera_config_t`.  
- Enables PSRAM for framebuffer storage.  

3. MJPEG Stream Handler
- Runs a loop capturing frames from the camera.  
- Sends each frame over HTTP as a multipart stream.  
- Browser interprets this as a continuous MJPEG stream.  

4. Web Server
- Starts an HTTP server and registers the `/stream` endpoint.  
- Accessible from any device on the same Wi-Fi network.

## Usage

1. Clone the repository:

```bash
git clone <repo-url>
cd <repo-directory>
```

2. Configure your Wi-Fi credentials in credentials.h:

```C
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"
```

3. Build and flash the code using ESP-IDF:
```bash
idf.py set-target esp32
idf.py menuconfig  # enable PSRAM
idf.py build flash monitor
```

4. Open a browser and navigate to:
```
http://<ESP_IP>/stream
```
<ESP_IP> is printed in the serial monitor after connecting to Wi-Fi.
## Exemple Setup
<p>
The setup is a bridal shoot for these two absolutely escquisite dolls.
The camera was held by the kind soldering fixator. A pink piece of yarn kept the camera in place.
This was the setup. The lamp was not present during the photoshoot. <br>
</p>
<p  align="center">
<img width="400" height="auto" alt="image" src="https://github.com/user-attachments/assets/46f43829-74b0-4c87-9e51-4a79cf8b46fc" />
</p>

## Results
Tested frame sizes are: QVGA, VGA, SVGA and XGA.<br>
JPEG qualities tested are:  0 20 40 60. <br>
The quality 0 always causes an overflow in the camera buffer.
<img width="2000" height="812" alt="image" src="https://github.com/user-attachments/assets/ea1ffeeb-abbe-40df-869d-a1d6d3518c82" />

