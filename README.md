# UberMayhem Portapack ESP32 module

## Introduction

The goal of the project is to create a simple add-on module for the PortaPack H4M based on an ESP32 S2-Mini. This module feels appropriate in size and already has most of the basic components on the circuit board to serve as a good foundation.

The source code for the ESP32 module will be written in Arduino IDE framework and adapted for PlatformIO.

The choice of the Arduino framework instead of Espressif ESP-IDF is due to my limited knowledge of the latter, as well as my feeling that there are more people out there in the community who are comfortable with Arduino than ESP-IDF.
It offers a lower barrier to entry and access to a wide range of libraries, making it suitable for hobby projects and rapid prototyping like this.

The project has no deadline and will only progress when motivation and opportunity arise. I also cannot guarantee that it will ever be completed. I see it more as a fun thing, and as long as it entertains me, I will continue.

## Ideas for this project

-   Control Portapack via web interface; view screen, access files, etc.
-   Retrieve GPS position including other available sensors from a mobile phone connecting to a webpage
-   Create an application for Portapack that can trigger various functions on the ESP32. The application should fetch available functions that the ESP32 has and present them on the Portapack.
-   IR blaster
- etc.
