#include <Arduino.h>

#include "uber_config.h"
#include "UberESP.h"
#include "UberPayload.h"
#include "UberPortapack.h"

UberESP uberESP;
UberPayload uberPayload;
UberPortapack uberPortapack;

#if FEATURE_MQTT
// -----------------------------------------------------------------------
// MQTT Publish entities
// -----------------------------------------------------------------------
void mqttPublishEntity(const char *sub_topic) {
  JsonDocument entityJson;
  std::string  entityText;

  // - - - - - - - - - - - - - - - - - - - - - - - - - 

  // Publish current state for light entity
  if (strcmp(sub_topic, "light") == 0) {
    // Reply state as Plaintext
    //entityText = light == true ? "ON" : "OFF";;

    // Reply state as JSON
    //entityJson["state"] = light == true ? "ON" : "OFF";
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - 

  // Create MQTT message from JSON or plaintext variable
  char jsonMessage[measureJson(entityJson) + 1];
  serializeJson(entityJson, jsonMessage, sizeof(jsonMessage));

  // Decide if we are going to send a JSON message or plaintext message
  if (strcmp(jsonMessage, "null") != 0) {
    // Send JSON message
    uberESP.mqttPublish(sub_topic, jsonMessage);
    uberESP.eventPrintf("MQTT|%s, json: %s", sub_topic, jsonMessage);
  } else {
    if (entityText.length() > 0) {
      // Send Plaintext message
      uberESP.mqttPublish(sub_topic, entityText.c_str());
      uberESP.eventPrintf("MQTT|%s, plaintext: %s", sub_topic, entityText);
    }
  }
}

// -----------------------------------------------------------------------
// MQTT onMessage callback
// -----------------------------------------------------------------------
// Handle incoming MQTT-message(s)
void onMqttMessage(const char* sub_topic, const char* message) {
  JsonDocument jsonDoc;
  auto jsonError = deserializeJson(jsonDoc, message);

   // - - - - - - - - - - - - - - - - - - - - - - - - - 

  // Parse subtopic for light entity
  if (strcmp(sub_topic, "light") == 0) {
    if (jsonError) {
      // Parse as plaintext
      /*
      if (strcasecmp(message, "on") == 0)
        light = true;
      else
        light = false;
      */
      uberESP.eventPrintf("MQTT|%s, plaintext: %s", sub_topic, message);
    } else if (!jsonError) {
      // Parse as JSON
      /*
      if ( jsonDoc["state"].is<const char*>() ) {
        if (strcasecmp(jsonDoc["state"], "on") == 0)
          light = true;
        else
          light = false;
      }
      */
      uberESP.eventPrintf("MQTT|%s, json: %s", sub_topic, message);
    }

    // Publish current (updated) state
    mqttPublishEntity("light");

  } else {
    // Unknown subtopic
    uberESP.eventPrintf("MQTT|unknown subtopic: %s", sub_topic);
  }
}

// -----------------------------------------------------------------------
// MQTT onConnect callback
// -----------------------------------------------------------------------
// When connected to MQTT-server
void onMqttConnect() {
  // Initialize incremental ID for each entity
  uberESP.mqtt_hass_autoconfig_id = 0;

  // Send initial autodiscovery config message to HASS
  //
  // <entity_type>, [sub topic], [measurement unit for sensor], [custom json config]
  uberESP.mqttPublishHassAutodiscover("light", "light", "", "");

  // Publish entity states at initial connect
  mqttPublishEntity("light");
}
#endif

// -----------------------------------------------------------------------
// Websocket frame callback
// -----------------------------------------------------------------------
// When a websocket frame (message) arrives
void onWsFrame(PsychicWebSocketRequest *request, httpd_ws_frame *frame) {
    char *data = (char*)frame->payload;

    // - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Incoming "set" commands that doesn't need any response

    // Syntax: setGps={"longitude":58.00,"latitude":12.0, ...}
    if (strncasecmp(data, "setGps=", 7) == 0)
      uberPortapack.setGpsDataJson(data + 7);

    // Syntax: setEnv={"temprature":21.50,"humidity":52.0, ...}
    else if (strncasecmp(data, "setEnv=", 7) == 0)
      uberPortapack.setEnvDataJson(data + 7);

    // Syntax: setOri={"angle":180.00,"tilt":12.0}
    else if (strncasecmp(data, "setOri=", 7) == 0)
      uberPortapack.setOriDataJson(data + 7);

    // Syntax: setLight={"light":120.00}
    else if (strncasecmp(data, "setLight=", 9) == 0)
      uberPortapack.setLightDataJson(data + 9);

    // Syntax: ppShell=<command>
    else if (strncasecmp(data, "ppShell=", 8) == 0)
      uberPortapack.sendShellCommand( const_cast<char*>( (std::string(data + 8) + "\r\n" ).c_str()) );

    // Syntax: setPayload={"name":"SSID RickRoll"}
    else if (strncasecmp(data, "setPayload=", 11) == 0)
      uberPayload.setJson(data + 11);

    // Syntax: setPayload={"name":"SSID RickRoll"}
    else if (strncasecmp(data, "setPayloadInterval=", 19) == 0) {
      uberPayload.interval = atoi(data + 19);
      Serial.println(uberPayload.interval); // Debug
    }

    // Syntax: togglePayload
    else if (strncasecmp(data, "togglePayload", 13) == 0)
      uberPayload.toggle();

    // Syntax: startPayload
    else if (strncasecmp(data, "startPayload", 12) == 0)
      uberPayload.start();

    // Syntax: stopPayload
    else if (strncasecmp(data, "stopPayload", 11) == 0)
      uberPayload.stop();

    // - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Incoming "get" commands that expects a response

    std::string command = data + 3; // Remove "get" part from the data
    std::string message;

    for (char& c : command) { c = std::tolower(c); }

    // Get GPS coordinates
    if (strcasecmp(data, "getGps") == 0)
      message = uberPortapack.getGpsDataJson();

    // Get Environment
    else if (strcasecmp(data, "getEnv") == 0)
      message = uberPortapack.getEnvDataJson();

    // Get Orientation
    else if (strcasecmp(data, "getOri") == 0)
      message = uberPortapack.getOriDataJson();

    // Get Light
    else if (strcasecmp(data, "getLight") == 0)
      message = uberPortapack.getLightDataJson();

    // Get available payloads
    else if (strcasecmp(data, "getPayloads") == 0)
      message = uberPayload.getPayloadsJson();

    // Get payload state
    else if (strcasecmp(data, "getPayloadState") == 0)
      message = uberPayload.getPayloadStateJson();

    // Reply to get-commands with an JSON array
    if (message.length() > 0)
      request->reply( ( "{\"" + command + "\":" + message + "}" ).c_str() );
}

// -----------------------------------------------------------------------
// Portapack shell data callback
// -----------------------------------------------------------------------
// When incoming shell data arrives
bool onPPShellData(const uint8_t* data, size_t data_len) {
      // Send all incoming shell data from PP to websocket
      uberESP.sendWsAll((const char*)data, data_len);

      vTaskDelay(1 / portTICK_PERIOD_MS);
      return true;
}

// -----------------------------------------------------------------------
// Payload State Change callback
// -----------------------------------------------------------------------
void onPayloadStateChange(bool state) {
  std::string message;
  message = uberPayload.getPayloadStateJson();
  uberESP.wsPrintf( "{\"payloadstate\":%s}", message.c_str() );
};

// -----------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------
void coreTask(void * pvParameters);

void setup(void) {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);

  psramInit();

  pinMode(0, INPUT_PULLUP);  // Button on ESP32-S2 mini
  pinMode(LED_PIN, OUTPUT);  // Onboard LED on ESP32-S2 mini
  digitalWrite(LED_PIN, 32); // Onboard LED brightness

  uberESP.init();

  // Only start if we are not in "safe mode"
  if (!uberESP.inSafemode) {
    #if FEATURE_MQTT
    uberESP.mqttStart();
    uberESP.setOnMqttConnectCallback(onMqttConnect);
    uberESP.setOnMqttMessageCallback(onMqttMessage);
    #endif

    #if FEATURE_MDNS
    uberESP.mdnsStart();
    #endif

    // Set callback for incoming websocket frames
    uberESP.setOnWsFrameCallback(onWsFrame);

    // Initialize UberPayload and set callback for payload state change
    uberPayload.init();
    uberPayload.setOnStateChangeCallback(onPayloadStateChange);

    // Initialize UberPortapack and set callback for incoming shell data
    uberPortapack.init(I2C_SDA, I2C_SCL, I2C_DEV_ADDR);
    uberPortapack.setOnShellDataCallback(onPPShellData);

    // Start coreTask pinned to Core #1
    xTaskCreatePinnedToCore(coreTask, "coreTask", 2048, NULL, 2, NULL, 1);
  }
}

// -----------------------------------------------------------------------
// Task process
// -----------------------------------------------------------------------
void coreTask(void * pvParameters) {
  int last_button_state = HIGH;

  // - - - - - - - - - - - - - - - - - - - - - - - - - 

  // Main loop
  while (1) {
    int button_state = digitalRead(0);

    // ESP32-S2 Mini button (GPIO0) toggles payload on/off
    if (button_state != last_button_state) {
      last_button_state = button_state;

      if (button_state == LOW) {
        //uberPayload.setJson("{'name':'SSID RickRoll'}");
        uberPayload.set(Payload::WIFI_SSID_DYXLISEA);
        uberPayload.toggle();
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// -----------------------------------------------------------------------

// Dummy Arduino-loop (unused)
void loop(void) { vTaskDelay(5000 / portTICK_PERIOD_MS); }