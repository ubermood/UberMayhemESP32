#ifndef UBERPAYLOAD_H
#define UBERPAYLOAD_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <unordered_map>

#define OFFSET_BEACON_DST_ADDR   4
#define OFFSET_BEACON_SRC_ADDR  10
#define OFFSET_BEACON_BSSID     16
#define OFFSET_BEACON_SEQNUM    22
#define OFFSET_BEACON_TIMESTAMP 24
#define OFFSET_BEACON_SSID      38

enum class Payload : uint8_t {
    WIFI_SSID_RICKROLL    = 1,
    WIFI_SSID_RANDOMALPHA = 2,
    WIFI_SSID_CLONIE      = 3,
    WIFI_SSID_CUTIE       = 4,
    WIFI_SSID_DYXLISEA    = 5,
};

class UberPayload {
  protected:

  private:
    Payload payload;

    // String to enum mapping for each payload
    const std::unordered_map<std::string, Payload> payloadMap = {
      {"SSID RickRoll",    Payload::WIFI_SSID_RICKROLL},
      {"SSID RandomAlpha", Payload::WIFI_SSID_RANDOMALPHA},
      {"SSID Clonie",      Payload::WIFI_SSID_CLONIE},
      {"SSID Cutie",       Payload::WIFI_SSID_CUTIE},
      {"SSID DyxLisea",    Payload::WIFI_SSID_DYXLISEA},
    };

    bool running;
    int led_value_before;

    static uint8_t beacon_raw[];

    static const char* rick_ssids[];

    uint32_t ssid_inc;
    uint32_t lcg_state;
    uint32_t lcgRand();

    void generateRandSsid(char* ssid, uint32_t seed, uint8_t length);
    void randomizeBeaconSrcMac(uint8_t* beacon_raw);
    void updateBeaconTimestamp(uint8_t *beacon_raw);

    void wifiSsidRickRoll();
    void wifiSsidRandomAlpha();
    void wifiSsidCloneSpam();

    void payloadTask(void *pvParameter);
    static void staticPayloadTask(void* pvParameter) {
      UberPayload* instance = static_cast<UberPayload*>(pvParameter);
      instance->payloadTask(pvParameter);
    }
    static void payloadTaskWrapper(void *param) {
      UberPayload *uberPayload = reinterpret_cast<UberPayload*>(param); // Obtain the instance pointer
      uberPayload->payloadTask(param); // Dispatch to the member function
    }

    void (*onPayloadStateChangeCallback)(bool);

    void notifyStateChangeCallback(bool state) {
        if (onStateChangeCallback != nullptr) {
            onStateChangeCallback(state);
        }
    }

  public:
    void init();

    void start();
    void toggle();
    void stop();
    bool isRunning();

    void (*onStateChangeCallback)(bool state);
    void setOnStateChangeCallback(void (*onStateChangeCallback)(bool state));

    Payload get();
    void set(Payload);
    void setJson(const char* json);

    std::string getPayloadsJson();
    std::string getPayloadStateJson();

    int interval;
};

#endif // UBERPAYLOAD_H