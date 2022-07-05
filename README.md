# Automatic-Door
Automatic Cabinet Door controlled by an ESP32, using a linear actuator and a distance sensor.

## Authors
Pedro Oliveira (Original Developer) & Jorge S. Calado (Supervisor)

## Hardware Requirements
- ESP32 microcontroller (or equivalent): [Espressif ESP32 PICO KIT V4](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-pico-kit.html)
- Distance/Proximity Sensor: [Ultrasonic Sensor HC-SR04](https://www.sparkfun.com/products/15569)
- Linear Actuator (200mm stroke): [Actuonix P16-P Linear Actuator with Feedback 200mm 22:1 12 volts](https://www.actuonix.com/p16-200-22-12-p)
- Linear Actuator Controller (LAC): [Actuonix Linear Actuator Control Board](https://www.actuonix.com/lac)
- Power Supply 12V-5A, to power the LAC
- Power Supply 5V-5A or a DC12V-5V Converter, to power the ESP32

## Required Libraries
- WiFi.h
- PubSubClient.h
- Ultrasonic.h
- ESP32Servo.h

## Other Requirements
- MQTT Broker for message queuing

## Installation
1. Connect the Linear Actuator to the LAC board:  
<img src="/images/lac%2Bla.png" alt="P-16" width="200"/> <img src="/images/p16-p-family.png" alt="P-16" width="400"/>  
2. Connect the LAC RC and GND(-) pins to the ESP32 (pin 27 and GND):  
<img src="/images/lac%2Besp.png" alt="P-16" width="200"/> <img />  
3. Connect the 12V VCC(+) and GND to the LAC:  
<img src="/images/lac%2Bpw.png" alt="P-16" width="200"/> <img />  
4. Connect the HC-SR04 VCC and GND pins to the ESP32 3V and GND outputs, and the TRIG and ECHO pins to the D25 and D26 data pins:  
<img src="/images/esp32-pico-kit-pinouts.png" alt="P-16" width="300"/> <img src="/images/HC-SR04.png" alt="P-16" width="200"/>  
5. Connect the VCC (5V) and GND to the ESP pins  
6. Verify all connections are correct and secure
7. Change the configurations and upload the code to the ESP  
8. Switch on the power sources -> WARNING: the door will open and close (once) on the system power-on!  

## Configurations
Door_Controller_ESP32.ino  
Change ```#define ACTUATOR_PIN 27``` to the data pin connected to the LAC  
Change ```const char* mqtt_server = "192.168.0.100";``` to the ip of the MQTT Broker  
Change ```const char *wifi_ssid = "SSID";``` to the WLAN SSID  
Change ```const char *wifi_password = "PASSWORD";``` to the WLAN Password  
Change ```Ultrasonic ultrasonic(25, 26);";``` to the TRIG (25) and ECHO (26) pins of the HC-SR04  
Change ```const int openZone = 130;";``` to the desired máximum proximity distance (cm) for door operation  
Change ```const int safeZone = 60;";``` to the desired minimum proximity distance (cm) for door operation (should be equal or greater than the lenght of the door)  

## Functioning
3 working modes:
  - "Automatic" - Door automaticaly opens (if presence detected between 60cm and 140cm), and closes (5 seconds after presence no longer detected);
  - "Manual" - Door opens/closes only on command (if presence detected between 60cm and 140cm/5 seconds after presence no longer detected);
  - "Open Mode" - Door opens/closes only on command regardless of distance to the door.
