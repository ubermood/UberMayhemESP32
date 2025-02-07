# UberMayhem Portapack ESP32 module

## Introduction

The goal of the project is to create a simple add-on module for the PortaPack H4M based on an ESP32 S2-Mini. This module feels appropriate in size and already has most of the basic components on the circuit board to serve as a good foundation.

The source code for the ESP32 module will be written in Arduino IDE framework and adapted for PlatformIO.

The choice of the Arduino framework instead of Espressif ESP-IDF is due to my limited knowledge of the latter, as well as my feeling that there are more people out there in the community who are comfortable with Arduino than ESP-IDF.
It offers a lower barrier to entry and access to a wide range of libraries, making it suitable for hobby projects and rapid prototyping like this.

The project has no deadline and will only progress when motivation and opportunity arise. I also cannot guarantee that it will ever be completed. I see it more as a fun thing, and as long as it entertains me, I will continue.

## Compile and install the firmware

* ***Step 1 (prerequisite):***

	Install **VisualStudio Code**<br>
	Install **PlatformIO IDE** extension in VS Code

* ***Step 2:***

	Clone this project:<br>
	``git clone https://github.com/ubermood/UberMayhemESP32.git``

	In PlatformIO, select **File -> Open Folder...** from the menu.<br>
	Browse to ``UberMayhemESP32/Source`` folder and click **Open** button.

	>Check ``lib/UberMayhem/uber_config.ini`` file and change the predefined config entries if needed (not mandatory).<br>
	>These are the initial, default config values used when no saved config exists in the ESP EEPROM.

* ***Step 3:***

	<ins>Flash firmware via Serial interface:</ins>
 
	**Build and Upload** the firmware with the USB-cable connected directly to your ESP32-device.

	OR...

	<ins>Flash firmware Over-The-Air:</ins>
 
	**Build** the **firmware.bin** file (can be found in the hidden ``.pio/...`` folder structure).<br>
	Upload that file from **UberMayhem ESP configuration** page, by pressing **Firmware Update** button<br>
	(if you already have UberMayhemESP32 flashed on your ESP-device since before)<br>

* ***Step 4:***
  
  Connect to the wireless AccessPoint SSID named **ESP_123ABC** with the password that you set earlier in ``uber_config.ini`` ..or use the default PW: "***password***".

  <ins>Browse to URL:</ins><br>
    http://10.0.0.1 - to access the ESP Configuration page<br>
    http://10.0.0.1/portapack - to access the PortaPack Command and Control Center

## Notes

PortaPack compapibility have only been fully tested on a **ESP32-S2 Mini** device.<br>

It should work with **ESP32-S3 SuperMini** and **ESP32-S3 DevKit** boards also, but has not been verified yet.<br>
It may even be preferred to use these boards, since they are slightly better, have more RAM etc.

>ESP32 board can be changed in ``platformio.ini`` file by changing ``default_envs = < ... >``.<br>
>There are alreay a couple of them listed and pre-made to test out.<br>
>
>You may also need to change the currently assigned I2C pins in ``uber_config.ini`` to match your specific setup.

## Web URLs

|URL|Description
|-|-
|/         |Main ESP configuration page
|/portapack|PortaPack "Command and Control Center"
|/load     |Get ESP configuration (responds with a JSON-string)
|/save     |Save ESP configuration (send a POST-request with all the parameters to change)
|/wifiScan |Scan for all available WIFI APs (responds with a JSON-string)
|/reboot   |Reboot the ESP device
|...       |Not documented

## WebSocket commands

***Sensors:***<br>
Get/Set sensor data currently stored on the ESP, that will be communicated via I2C interface to the PortaPack whenever it requests for it.

***Payloads:***<br>
Payloads are functions available on the ESP-device that can be executed.<br>
There are currently only SSID beacon spam related stuff available, but the list may be extended in the future.

***Portapack:***<br>
PortaPack specific commands.

***ESP:***<br>
ESP specific commands.

---

||Command|Description
|-|-|-
|**Sensors**|getGps|Get GPS data known by ESP and shared with PortaPack
||getOri           |Get Orientation data known by ESP and shared with PortaPack
||getEnv           |Get Environment data known by ESP and shared with PortaPack
||getLight         |Get Light data known by ESP and shared with PortaPack
||setGps={ `...json...` }  |Send GPS data to ESP
||setOri={ `...json...` }  |Send Orientation data to ESP
||setEnv={ `...json...` }  |Send Eniroment data to ESP
||setLight={ `...json...` }|Send Light data to ESP
|**Portapack**|ppShell=`command` |Run a PortaPack ChibiOS shell-command. (send "help" for more info)
|**Payloads**|getPayloads        |Get all available payload functions on ESP
||getPayloadState     |Get current payload state, ex. {"payloadstate":{"name":"`payload name`", "state":"`running / stopped`"}}
||setPayload={"name":"`payload name`"}|Set the payload that will be used
||startPayload        |Start currently set payload
||stopPayload         |Stop a payload from running
||togglePayload       |Toggle between start / stop
|**ESP**| getSysinfo  |Get ESP system information
|| getConfig          |Get ESP configuration (responds with same data as URL: /load)
|| reboot             |Reboot ESP device

## Known issues

- Some "payloads" relies on the APs that have been found after a wireless scan.<br>
  The scan is only done once - when you access the config page (URL: /), to populate the AP dropdown selector

- File Upload & Edit functions have not been implemented yet

- If more than 1 client is connected to the webpage, websocket communication interference happens (especially on downloads and screen updates)

- Experimental HTTPS support is not working on ESP32-S2 Mini, propably since it has not enough memory (Utilize PSRAM?).<br>
  Workaround is to enable some Chrome-flags (hints are shown as a banner when accessing "/portapack" URL).<br>
  The ESP32-S3 boards may work better...

## Screenshot (sensors, console & file browser)
![PortaPack_Screenshot1](https://github.com/user-attachments/assets/2c9b018d-c21a-4518-889c-0796bb0e53d8)

## Screenshot (payloads, screen & file browser)
![PortaPack_Screenshot2](https://github.com/user-attachments/assets/27a51c0b-73ed-47f4-9f83-1df552cc4616)
