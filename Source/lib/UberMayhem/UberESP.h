#include "uber_config.h"

#include <esp_wifi.h>
#include <WiFi.h>
#include <Update.h>
#include "AsyncTCP.h"

#include <DNSServer.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <StreamString.h>

#include <PsychicHttp.h>
#if FEATURE_SSL
#include <PsychicHttpsServer.h>
#endif

#if FEATURE_MDNS
#include <ESPmDNS.h>
#endif

#if FEATURE_MQTT
#include <AsyncMqttClient.h>
#endif

// - - - - - - - - - - - - - - - 

#ifndef CONF_VERSION
#define CONF_VERSION     "N/A"
#endif

#ifndef CONF_VERSION_MIN
#define CONF_VERSION_MIN "N/A"
#endif

#ifndef CONF_DEVICE_DESC
#define CONF_DEVICE_DESC "ESP32 Device"
#endif

#ifndef CONF_WWW_AUTH
#define CONF_WWW_AUTH    false
#endif

#ifndef CONF_WWW_USER
#define CONF_WWW_USER    "admin"
#endif

#ifndef CONF_WWW_PASS
#define CONF_WWW_PASS    "password"
#endif

#ifndef CONF_WIFI_SSID
#define CONF_WIFI_SSID   "SSID"
#endif

#ifndef CONF_WIFI_PASS
#define CONF_WIFI_PASS   "password"
#endif

#ifndef CONF_WIFI_AP_PASS
#define CONF_WIFI_AP_PASS "password"
#endif

#ifndef CONF_WIFI_DHCP
#define CONF_WIFI_DHCP   true
#endif

#ifndef CONF_MQTT_SERVER
#define CONF_MQTT_SERVER "some-mqtt-server.com"
#endif

#ifndef CONF_MQTT_PORT
#define CONF_MQTT_PORT   1883
#endif

#ifndef CONF_MQTT_USER
#define CONF_MQTT_USER   ""
#endif

#ifndef CONF_MQTT_PASS
#define CONF_MQTT_PASS   ""
#endif

#ifndef CONF_MQTT_HASS
#define CONF_MQTT_HASS   true
#endif

#ifndef CONF_MQTT_TOPIC_HASS
#define CONF_MQTT_TOPIC_HASS "homeassistant"
#endif

#ifndef CONF_MQTT_TOPIC_ROOT
#define CONF_MQTT_TOPIC_ROOT "esp32"
#endif

#ifndef CONF_MQTT_TOPIC_SUB
#define CONF_MQTT_TOPIC_SUB	 "set"
#endif

#ifndef CONF_MQTT_TOPIC_PUB
#define CONF_MQTT_TOPIC_PUB  ""
#endif

#ifndef CONF_COLORMODE
#define CONF_COLORMODE	     "rgb"
#endif

#ifndef CONF_LED_EFFECTS
#define CONF_LED_EFFECTS     "\"solid\""
#endif

#ifndef CONF_LED_BRIGHTNESS
#define CONF_LED_BRIGHTNESS  70
#endif

#ifndef CONF_LED_COLOR
#define CONF_LED_COLOR       "FF6417"
#endif

#ifndef CONF_LED_COUNT
#define CONF_LED_COUNT       1
#endif

#ifndef CONF_LED_PIN
#define CONF_LED_PIN         5
#endif

// Timeout for safemode survival test and EEPROM address where to save state
#define SAFEMODE_TIMEOUT 3
#define SAFEMODE_ADDRESS 512-1

// - - - - - - - - - - - - - - - 

class UberESP {
  protected:

  static void wifiConnectWrapper(void *param) {
    UberESP *uberESP = reinterpret_cast<UberESP*>(param); // Obtain the instance pointer

#ifdef CONF_WIFI_POWER
    // Set WiFi TX power
    WiFi.setTxPower(CONF_WIFI_POWER);
#endif

    while (1) {
      uberESP->wifiConnect(); // Dispatch to the member function, now that we have an instance pointer
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  };

  private:
#if FEATURE_MQTT
  AsyncMqttClient  mqttClient;
  Ticker           mqttReconnectTimer;
#endif
  Ticker           rebootTimer;
  Ticker           safemodeTimer;
  Ticker           wifiConnectTimer;
  DNSServer        DNS;  

  unsigned long wifi_disconnect_timer = 0;
  unsigned long wifi_reconnect_timer = 0;

  public:
  typedef struct { 
    char      deviceName[16];
    char      deviceDesc[32];
    bool      wwwAuth;
    char      wwwUser[8];
    char      wwwPass[16];
    char      wifiSSID[32];
    char      wifiPass[32];
    bool      netDHCP;
    IPAddress netIP;
    IPAddress netMask;
    IPAddress netGW;
    IPAddress netDNS;
    char      mqttServer[64];
    uint16_t  mqttPort;
    char      mqttUser[16];
    char      mqttPass[16];
    bool      mqttHass;
    char      mqttTopicHass[16];
    char      mqttTopicRoot[16];
    char      mqttTopicSub[16];
    char      mqttTopicPub[16];
    char      colorMode[4];
    uint8_t   ledBrightness;
    char      ledColor[7];
    uint16_t  ledCount;
    uint8_t   ledPin;
    char      check[5];
  } Config;

  // Default config

  Config config = {
    "",
    CONF_DEVICE_DESC,
    CONF_WWW_AUTH,
    CONF_WWW_USER,
    CONF_WWW_PASS,
    CONF_WIFI_SSID,
    CONF_WIFI_PASS,
    CONF_WIFI_DHCP,
    IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0),
    CONF_MQTT_SERVER,
    CONF_MQTT_PORT,
    CONF_MQTT_USER,
    CONF_MQTT_PASS,
    CONF_MQTT_HASS,
    CONF_MQTT_TOPIC_HASS,
    CONF_MQTT_TOPIC_ROOT,
    CONF_MQTT_TOPIC_SUB,
    CONF_MQTT_TOPIC_PUB,
    CONF_COLORMODE,
    CONF_LED_BRIGHTNESS,
    CONF_LED_COLOR,
    CONF_LED_COUNT,
    CONF_LED_PIN,
    "CONF"
  };

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  bool inSafemode = false;
  bool mqttConfigured = false;
  bool wifiConnecting = false;

  // Safemode status
  byte safemode_status;

  // Global variables used by methods, generated when mqttStart() is called
  char mqtt_fulltopic_root[16+16+8];     // esp32 [16]/ <config.deviceName> [16]
  char mqtt_fulltopic_sub[16+16+16+16];  // esp32 [16]/ <config.deviceName> [16]/ <get...> [16]
  char mqtt_fulltopic_pub[16+16+16+16];  // esp32 [16]/ <config.deviceName> [16]/ <set...> [16]

  // Hass autoconfig id counter, increasing each time autoconfig is called
  uint8_t mqtt_hass_autoconfig_id = 0;

  // User defined callback functions
  void (*onMqttConnectCallback)();
  void (*onMqttMessageCallback)(const char *, const char *);
  void (*onWsFrameCallback)(PsychicWebSocketRequest *request, httpd_ws_frame *frame);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void init();
  void configLoad();
  void configSave();
  bool safemodeCheck();
  void wifiStart();
  void wifiConnect();
  void mdnsStart();
  void mdnsHandler();
  void httpStart();

  TaskHandle_t taskWifiConnect;
  void onWifiEvent(WiFiEvent_t event);

  std::string getConfig();
  std::string getSysinfo();
  void        setConfig(JsonVariant &json);
  void        reboot();
  std::string wifiScan();
  esp_err_t   webpageIndex(PsychicRequest *request);
  esp_err_t   webpageUpdateRequest(PsychicRequest *request);
  esp_err_t   webpageUpdateUpload(PsychicRequest *request, const String& filename, uint64_t index, uint8_t *data, size_t len, bool final);
  esp_err_t   webpagePortapack(PsychicRequest *request);
  void        setOnWsFrameCallback(void (*onWsFrameCallback)(PsychicWebSocketRequest *request, httpd_ws_frame *frame));

#if FEATURE_MQTT
  void mqttConnect();
  void mqttStart();
  void mqttPublishHassAutodiscover(const char* entity_type, const char* sub_topic, const char* unit, const char* custom_json);
  void mqttPublish(const char* sub_topic, const char* message);
  void mqttPublishStartup();
  void onMqttConnect(bool sessionPresent);
  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
  void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total);
  void setOnMqttConnectCallback(void (*onMqttConnectCallback)());
  void setOnMqttMessageCallback(void (*onMqttParseCallback)(const char *, const char *));
#endif

  void sendWsAll(const char *message, size_t len);
  void wsPrintf(const char *format, ...);
  void eventPrintf(const char *format, ...);
  void eventPrint(const char* message);

  std::string getUpdateError();
};