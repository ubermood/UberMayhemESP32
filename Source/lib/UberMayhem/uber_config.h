#define CONF_VERSION       CONF_VERSION_MIN " (UberMayhem for Portapack H4M by ubermood)"
#define CONF_VERSION_MIN   "20250226"         // Minified version number
#define CONF_DEVICE_DESC   "UberMayhem"       // Shown as entity name in HASS

#define DEBUG              true               // Serial Debug messages
#define FEATURE_MQTT       false              // Enable MQTT
#define FEATURE_MDNS       false              // Enable MDNS
#define FEATURE_SSL        false              // Enable HTTPS (experimental!)

#define CONF_WWW_USER      "admin"            // Default username for Web UI
#define CONF_WWW_PASS      "password"         // Default password for Web UI

#define CONF_WIFI_SSID     "wifi_ssid"        // Default WIFI SSID when unconfigured
#define CONF_WIFI_PASS     "wifi_ssid_pass"   // Default WIFI SSID password when unconfigured
#define CONF_WIFI_AP_PASS  "password"         // Password to internal WIFI AP SSID
#define CONF_WIFI_AP_STOP  false              // Stop internal AP on successful connect to remote AP
//#define CONF_WIFI_POWER  WIFI_POWER_8_5dBm  // Set Wifi TX Power (Workaround! - see: https://github.com/espressif/arduino-esp32/issues/6767)

#define CONF_MQTT_SERVER   "mqtt-server.loc"  // Default MQTT server to use

#define I2C_SDA             9                 // I2C SDA pin 9 / 21
#define I2C_SCL             7                 // I2C SCL pin 7 / 22
#define I2C_DEV_ADDR        0x51              // I2C Device address for this ESP-module

// Defined automatically in platformio.ini
//#define LED_PIN          2                  // Define on-board LED pin (ESP32-DevBoard)
//#define LED_PIN          15                 // Define on-board LED pin (ESP32-S2-Mini)
//#define LED_PIN          48                 // Define on-board LED pin (ESP32-S3-DevBoard)
//#define LED_PIN          48                 // Define on-board LED pin (ESP32-S3-Super-Mini)