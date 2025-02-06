#ifndef UBERPORTAPACK_H
#define UBERPORTAPACK_H

// Fast mode = 400 Kbit/s, Standard mode = 100 Kbit/s
#define I2C_FREQ 400000

// Portapack-ArduinoIDE library
#include "pp_handler.h"
#include "pp_shellcomm_nousb.h"
#include "pp_ext_apps/pp_app_digitalrain.h"

class UberPortapack {
  protected:

  private:
    std::function<bool(const uint8_t*, size_t)> shellDataCallback;
    static UberPortapack* instance;

    void setCallbacks();

    static bool staticShellDataCallback(const uint8_t* data, size_t data_len) {
        // Call the instance method using a stored instance reference
        if (instance && instance->shellDataCallback) {
            return instance->shellDataCallback(data, data_len);
        }
        return false;
    }

  public:
    UberPortapack() {
        instance = this; // Set static instance
    }

    static ppgpssmall_t gps_data;
    static environment_t env_data;
    static orientation_t ori_data;
    static uint16_t light_data;

    void init(int i2c_sda, int i2c_scl, uint8_t i2c_dev_addr);
    void sendShellCommand(char* command);

    void setGpsDataJson(const char* json);
    void setEnvDataJson(const char* json);
    void setOriDataJson(const char* json);
    void setLightDataJson(const char* json);

    std::string getGpsDataJson();
    std::string getEnvDataJson();
    std::string getOriDataJson();
    std::string getLightDataJson();

    void (*onPPShellCallback)();
    void setOnShellDataCallback(std::function<bool(const uint8_t*, size_t)> callback) {
        shellDataCallback = callback;
    }
};

#endif // UBERPORTAPACK_H