#include "UberPortapack.h"
#include "ArduinoJson.h"

ppgpssmall_t UberPortapack::gps_data = {
  .latitude = 0.0,
  .longitude = 0.0,
  .altitude = 0.0,
  .sats_in_use = 0,
  .sats_in_view = 0,
  .speed = 0.0,
  .date = { 1, 1, 2025 },
  .tim = { 0, 0, 0 }
};

environment_t UberPortapack::env_data = {
  .temperature = 0.0,
  .humidity = 0.0,
  .pressure = 0.0
};

orientation_t UberPortapack::ori_data = {
  .angle = 0.0,
  .tilt = 0.0,
};

uint16_t UberPortapack::light_data = 0;

UberPortapack* UberPortapack::instance = nullptr;

void UberPortapack::init(int i2c_sda, int i2c_scl, uint8_t i2c_dev_addr) {
  // Set module parameters
  PPHandler::set_module_name("UberMayhem module");
  PPHandler::set_module_version(1);

  setCallbacks();

  // Initialize PPHandler I2C and PPShellComm
  PPHandler::init(i2c_sda, i2c_scl, i2c_dev_addr, I2C_FREQ);
  PPShellComm::init();

  // Set I2C connection state to true
  PPShellComm::set_i2c_connected(true);
}

void UberPortapack::sendShellCommand(char* command) {
  if ( !PPShellComm::getInCommand() ) {
    PPShellComm::write_blocking((uint8_t*)command, strlen(command));
  }
}

void UberPortapack::setCallbacks() {
  // Set supported features
  PPHandler::set_get_features_CB([](uint64_t& feat) {
    ChipFeatures chipFeatures{};
    chipFeatures.reset();
    chipFeatures.enableFeature(SupportedFeatures::FEAT_GPS);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_ORIENTATION);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_ENVIRONMENT);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_LIGHT);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_UART);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_SHELL);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_EXT_APP);
    chipFeatures.enableFeature(SupportedFeatures::FEAT_DISPLAY);

    feat = chipFeatures.getFeatures();
  });

  // Set sensor callbacks
  PPHandler::set_get_gps_data_CB([](ppgpssmall_t& gps) { gps = gps_data; });
  PPHandler::set_get_environment_data_CB([](environment_t& env) { env = env_data; });
  PPHandler::set_get_orientation_data_CB([](orientation_t& ori) { ori = ori_data; });
  PPHandler::set_get_light_data_CB([](uint16_t& light) { light = light_data; });

  // Add apps
  PPHandler::add_app((uint8_t*)pp_app_digitalrain, sizeof(pp_app_digitalrain));
  
  // Get TX queue size (ESP -> PP)
  PPHandler::set_get_shell_data_size_CB( []() -> uint16_t {
    return PPShellComm::get_i2c_tx_queue_size();
  });

  // Data coming from shell (PP -> ESP))
  PPHandler::set_got_shell_data_CB( [](std::vector<uint8_t>& data) {
    I2CQueueMessage_t msg;

    msg.size = data.size();
    size_t size = data.size();

    if (size > 64)
      size = 64;

    memcpy(msg.data, data.data(), size);

    auto ttt = pdFALSE;
    xQueueSendFromISR(PPShellComm::datain_queue, &msg, &ttt);
  });

  // Sent data to shell (ESP -> PP)
  PPHandler::set_send_shell_data_CB( [](std::vector<uint8_t>& data, bool& hasmore) {    
    hasmore = false;
    data.resize(64); 
    size_t size = PPShellComm::get_i2c_tx_queue_size();

    if (size > 64) {
        hasmore = true;
        size = 64;
    }

    // Copy from PPShellComm queue to data
    memcpy(data.data(), PPShellComm::get_i2c_tx_queue_data(), size);
    PPShellComm::clear_tx_queue();
  });

  // Data coming from shell (PP -> ESP)
  PPShellComm::set_data_rx_callback(UberPortapack::staticShellDataCallback);
}

void UberPortapack::setGpsDataJson(const char* json) {
    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, json);

    if ( jsonDoc["latitude" ].is<float>() ) { gps_data.latitude = jsonDoc["latitude"]; }
    if ( jsonDoc["longitude"].is<float>() ) { gps_data.longitude = jsonDoc["longitude"]; }
    if ( jsonDoc["altitude" ].is<float>() ) { gps_data.altitude = jsonDoc["altitude"]; }
    if ( jsonDoc["speed"    ].is<float>() ) { gps_data.speed = jsonDoc["speed"]; }
    if ( jsonDoc["day"      ].is<uint8_t>() ) { gps_data.date.day = jsonDoc["day"]; }
    if ( jsonDoc["month"    ].is<uint8_t>() ) { gps_data.date.month = jsonDoc["month"]; }
    if ( jsonDoc["year"     ].is<uint16_t>() ) { gps_data.date.year = jsonDoc["year"]; }
    if ( jsonDoc["hour"     ].is<uint8_t>() ) { gps_data.tim.hour = jsonDoc["hour"]; }
    if ( jsonDoc["minute"   ].is<uint8_t>() ) { gps_data.tim.minute = jsonDoc["minute"]; }
    if ( jsonDoc["second"   ].is<uint8_t>() ) { gps_data.tim.second = jsonDoc["second"]; }
    if ( jsonDoc["thousand" ].is<uint16_t>() ) { gps_data.tim.thousand = jsonDoc["thousand"]; }
}

void UberPortapack::setEnvDataJson(const char* json) {
    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, json);

    if ( jsonDoc["temperature"].is<float>() ) { env_data.temperature = jsonDoc["temperature"]; }
    if ( jsonDoc["humidity"   ].is<float>() ) { env_data.humidity = jsonDoc["humidity"]; }
    if ( jsonDoc["pressure"   ].is<float>() ) { env_data.pressure = jsonDoc["pressure"]; }
}

void UberPortapack::setOriDataJson(const char* json) {
    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, json);

    if ( jsonDoc["angle"].is<float>() ) { ori_data.angle = jsonDoc["angle"]; }
    if ( jsonDoc["tilt" ].is<float>() ) { ori_data.tilt = jsonDoc["tilt"]; }
    //if ( jsonDoc["roll"    ].is<float>() ) { ori_data.roll = jsonDoc["roll"]; }
    //if ( jsonDoc["pitch"   ].is<float>() ) { ori_data.pitch = jsonDoc["pitch"]; }
}

void UberPortapack::setLightDataJson(const char* json) {
    JsonDocument jsonDoc;
    deserializeJson(jsonDoc, json);

    if ( jsonDoc["light"].is<uint16_t>() ) { light_data = jsonDoc["light"]; }
}

std::string UberPortapack::getGpsDataJson() {
    JsonDocument jsonDoc;
    std::string message;

    jsonDoc["latitude" ] = gps_data.latitude;
    jsonDoc["longitude"] = gps_data.longitude;
    jsonDoc["day"      ] = gps_data.date.day;
    jsonDoc["month"    ] = gps_data.date.month;
    jsonDoc["year"     ] = gps_data.date.year;

    serializeJson(jsonDoc, message);
    return message;
}

std::string UberPortapack::getEnvDataJson() {
    JsonDocument jsonDoc;
    std::string message;

    jsonDoc["temperature"] = env_data.temperature;
    jsonDoc["humidity"   ] = env_data.humidity;
    jsonDoc["pressure"   ] = env_data.pressure;

    serializeJson(jsonDoc, message);
    return message;
}

std::string UberPortapack::getOriDataJson() {
    JsonDocument jsonDoc;
    std::string message;

    jsonDoc["angle"] = ori_data.angle;
    jsonDoc["tilt" ] = ori_data.tilt;

    serializeJson(jsonDoc, message);
    return message;
}

std::string UberPortapack::getLightDataJson() {
    JsonDocument jsonDoc;
    std::string message;

    jsonDoc["light"] = light_data;

    serializeJson(jsonDoc, message);
    return message;
}