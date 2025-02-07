#include "UberPayload.h"
#include "ArduinoJson.h"

// Bypass 802.11 RAW frames sanity check
extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  return 0;
}

// Beacon frame - will be used as template
uint8_t UberPayload::beacon_raw[] = {
    0x80, 0x00,                                                 // 00-01: Frame Control - Control (0b1000), Subtype: Beacon (0b0000)
    0x00, 0x00,                                                 // 02-03: Duration ID
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,                         // 04-09: Destination address (broadcast)
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,                         // 10-15: Source address
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06,                         // 16-21: BSSID
    0x00, 0x00,                                                 // 22-23: Sequence / fragment number
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // 24-31: Timestamp
    0x64, 0x00,                                                 // 32-33: Beacon interval (100)
    0x31, 0x04,                                                 // 34-35: Capability info

    0x00,                                                       // 36   : ElementID #0: SSID
    0x00,                                                       // 37   : Length (SSID hidden for simplicity)
    /* SSID name will be inserted here */                       // 38-xx: SSID

    0x01,                                                       // 38   : ElementID #1: Supported rates
    0x08,                                                       // 39   : Length
    0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,             // 40-47: Supported rates

    0x03,                                                       // 48   : ElementID #3: DS Parameter set
    0x01,                                                       // 49   : Length
    0x06,                                                       // 50   : Channel

    0x05,                                                       // 51   : ElementID #5: Traffic indicateion map (TIM)
    0x04,                                                       // 52   : Length
    0x01, 0x02, 0x00, 0x00,                                     // 53-56: 

    0x30,                                                       // 57   : ElementID #48: RSN information
    0x18,                                                       // 58   : Length
    0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x02, 0x00, 0x00,       // 59-84: WPA2 with CCMP
    0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04, 0x01, 0x00,
    0x00, 0x0f, 0xac, 0x02, 0x00, 0x00,
};

const char* UberPayload::rick_ssids[] = {
    "01 - Never gonna give you up",
    "02 - Never gonna let you down",
    "03 - Never gonna run around",
    "04 - and desert you",
    "05 - Never gonna make you cry",
    "06 - Never gonna say goodbye",
    "07 - Never gonna tell a lie",
    "08 - and hurt you"
};

const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void UberPayload::init() {
    running  = false;
    interval = 1;
    payload  = (Payload)1;

    xTaskCreate(
        &UberPayload::payloadTaskWrapper,
        "payloadTask",
        4096,
        this,
        5,
        NULL
    );
}

void UberPayload::setOnStateChangeCallback(void (*onStateChangeCallback_)(bool state)) {
    onStateChangeCallback = onStateChangeCallback_;
};

Payload UberPayload::get() {
    return payload;
}

void UberPayload::set(Payload payload_) {
    payload = payload_;
}

void UberPayload::setJson(const char* json) {
        JsonDocument jsonDoc;
        deserializeJson(jsonDoc, json);

        auto it = payloadMap.find(jsonDoc["name"]);
        if (it != payloadMap.end())
            payload = it->second;
}

void UberPayload::toggle() {
    !running ? start() : stop();
}

void UberPayload::start() {
    if (!running) {        
        lcg_state = 1;
        ssid_inc = 0;

        // Set random source MAC address
        randomizeBeaconSrcMac(beacon_raw);

        led_value_before = digitalRead(LED_PIN);
        running = true;

        notifyStateChangeCallback(running);
    }
}

void UberPayload::stop() {
    if (running) {
        running = false;

        // Ensure task has stopped before stopping blink
        vTaskDelay(250 / portTICK_PERIOD_MS);
        digitalWrite(LED_PIN, led_value_before);

        notifyStateChangeCallback(running);
    }
}

bool UberPayload::isRunning() {
    return running;
}

std::string UberPayload::getPayloadsJson() {
    JsonDocument jsonDoc;
    std::string message;

    // Create JSON string with buttons from payloadMap
    uint8_t i = 1;
    for (const auto& pair : payloadMap) {
        jsonDoc["btn" + String(i)] = pair.first;
        i++;
    }

    // Format, TODO - maybe..
    //
    //   {'payloads':
    //     { 'btn1': 'name',
    //       'txt1': 'name',
    //       'sel1': { 'name': ['opt0', 'opt2', 'opt3'] }
    //     }
    //   }

    // Simple button - Test
    //jsonDoc["btn1"] = "SSID RickRoll";
    //jsonDoc["btn2"] = "SSID RandomAlpha";

    // Input fields
    //jsonDoc["txt1"] = "SSID Fake";

    // Select options
    //JsonObject sel1 = jsonDoc["sel1"].to<JsonObject>();
    //sel1["SSID Spam"][0] = "RickRoll";
    //sel1["SSID Spam"][1] = "Random A-Z";
    //sel1["SSID Spam"][2] = "Random HEX";

    serializeJson(jsonDoc, message);
    return message;
}

std::string UberPayload::getPayloadStateJson() {
    JsonDocument jsonDoc;
    std::string message;

    // Find name of currently set payload
    std::unordered_map<Payload, const char*> reversePayloadMap;
    for (const auto& pair : payloadMap)
        reversePayloadMap[pair.second] = pair.first.c_str();

    jsonDoc["name"] = reversePayloadMap[payload];
    jsonDoc["state"] = isRunning() ? "started" : "stopped";

    serializeJson(jsonDoc, message);
    return message;
}

uint32_t UberPayload::lcgRand() {
    lcg_state = (uint64_t)1103515245 * lcg_state + 12345;
    return (lcg_state >> 16) & 0x7FFF;
}

void UberPayload::generateRandSsid(char* ssid, uint32_t seed, uint8_t length) {
    lcg_state = seed;  // Set the seed
    
    for (int i = 0; i < length; i++)
        ssid[i] = charset[lcgRand() % (sizeof(charset) - 1)];

    ssid[length] = '\0';
}

void UberPayload::randomizeBeaconSrcMac(uint8_t* beacon_raw) {
    // Generate and copy a 6-byte random MAC address to both Source Address and BSSID
    uint8_t random_byte;

    random_byte = random(1, 13);
    beacon_raw[50] = random_byte; // Channel

    for (int i = 0; i < 6; i++) {
        random_byte = random(0x00, 0xFF);
        beacon_raw[10 + i] = random_byte; // Source Address (bytes 10–15)
        beacon_raw[16 + i] = random_byte; // BSSID (bytes 16–21)
    }

    // Ensure the MAC address is valid (set locally administered bit, clear multicast bit)
    beacon_raw[10] = (beacon_raw[10] & 0xFE) | 0x02;
    beacon_raw[16] = beacon_raw[10]; // First byte of BSSID matches Source Address
}

void UberPayload::updateBeaconTimestamp(uint8_t *beacon_raw) {
    unsigned long millis_val = millis();

    // Convert millis_val to little-endian byte array
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 0] = (millis_val >> 24) & 0xFF;
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 1] = (millis_val >> 16) & 0xFF;
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 2] = (millis_val >> 8) & 0xFF;
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 3] = millis_val & 0xFF;
/*
    // Set the remaining bytes to zero
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 4] = 0;
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 5] = 0;
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 6] = 0;
    beacon_raw[OFFSET_BEACON_TIMESTAMP + 7] = 0;
*/
}

void UberPayload::wifiSsidRickRoll() {
    // SSID incremental counter
    ssid_inc = (ssid_inc + 1) % (sizeof(rick_ssids) / sizeof(char*));

    // Build the beacon frame
    uint8_t beacon[sizeof(beacon_raw) + 32];  // Max SSID length 32
    memcpy(beacon, beacon_raw, OFFSET_BEACON_SSID - 1);
    beacon[OFFSET_BEACON_SSID - 1] = strlen(rick_ssids[ssid_inc]);
    memcpy(&beacon[OFFSET_BEACON_SSID], rick_ssids[ssid_inc], strlen(rick_ssids[ssid_inc]));
    memcpy(&beacon[OFFSET_BEACON_SSID + strlen(rick_ssids[ssid_inc])], &beacon_raw[OFFSET_BEACON_SSID], sizeof(beacon_raw) - OFFSET_BEACON_SSID);

    // Incremental source MAC address for each SSID
    beacon[OFFSET_BEACON_SRC_ADDR + 5] = ssid_inc;
    beacon[OFFSET_BEACON_BSSID + 5] = ssid_inc;

    // Send 12-bit random sequence number
    uint16_t seq = esp_random() & 0xFFF;
    beacon[OFFSET_BEACON_SEQNUM] = (seq << 4) & 0xF0;
    beacon[OFFSET_BEACON_SEQNUM + 1] = (seq >> 4) & 0xFF;

    esp_wifi_80211_tx(WIFI_IF_AP, beacon, sizeof(beacon_raw) + strlen(rick_ssids[ssid_inc]), false);
}

void UberPayload::wifiSsidRandomAlpha() {
    // SSID incremental counter
    ssid_inc = (ssid_inc + 1) % 50;

    // Generate 32 pseudo-random SSIDs
    char ssid[33];
    generateRandSsid(ssid, ssid_inc, 24); // 24 random characters length of SSID

    // Build the beacon frame
    uint8_t beacon[sizeof(beacon_raw) + 32];  // Max SSID length 32
    memcpy(beacon, beacon_raw, OFFSET_BEACON_SSID - 1);
    beacon[OFFSET_BEACON_SSID - 1] = strlen(ssid);
    memcpy(&beacon[OFFSET_BEACON_SSID], ssid, strlen(ssid));
    memcpy(&beacon[OFFSET_BEACON_SSID + strlen(ssid)], &beacon_raw[OFFSET_BEACON_SSID], sizeof(beacon_raw) - OFFSET_BEACON_SSID);

    // Send 12-bit random sequence number
    uint16_t seq = esp_random() & 0xFFF;
    beacon[OFFSET_BEACON_SEQNUM] = (seq << 4) & 0xF0;
    beacon[OFFSET_BEACON_SEQNUM + 1] = (seq >> 4) & 0xFF;

    esp_wifi_80211_tx(WIFI_IF_AP, beacon, sizeof(beacon_raw) + strlen(ssid), false);
}

void UberPayload::wifiSsidCloneSpam() {
    int16_t network_cnt = WiFi.scanComplete();

    if (network_cnt >= 0) {
        // SSID incremental counter
        ssid_inc = (ssid_inc + 1) % network_cnt;

        // Pick next SSID from list
        String ssid_old = WiFi.SSID(ssid_inc);
        size_t ssid_old_length = ssid_old.length();

        char ssid[32 + 1];

        // ----

        // Cutie - Append a random emoji after each SSID
        if (payload == Payload::WIFI_SSID_CUTIE) {
            if (ssid_old_length < 32 - 6) {
                // https://apps.timwhitlock.info/emoji/tables/unicode

                ssid_old += " \xF0\x9F\x90";
                uint8_t random_emoji = random(0x8C, 0xBE);  // Random animal emoji
                ssid_old += char(random_emoji);
            }
            else { return; }

            ssid_old.toCharArray(ssid, sizeof(ssid));
        }

        // ----

        // Clonie - Append non-standard spaces after each SSID
        if (payload == Payload::WIFI_SSID_CLONIE) {
            // Ensure the SSID doesn't exceed 24 characters
            if (ssid_old_length >= 32) { return; }

            strncpy(ssid, ssid_old.c_str(), ssid_old_length);

            // Add random spaces after the SSID name
            size_t max_spaces = 32 - ssid_old_length;               // Remaining space for spaces
            size_t num_spaces = random(0, max_spaces + 1);          // Random number of spaces
            memset(ssid + ssid_old_length, char(0xA0), num_spaces); // Fill with non-standard spaces
            ssid[ssid_old_length + num_spaces] = '\0';              // Null-terminate the string
        }

        // ----

        // DyxLisea - Shuffle SSID characters around
        if (payload == Payload::WIFI_SSID_DYXLISEA) {
            ssid_old.toCharArray(ssid, sizeof(ssid));

            // Collect indices of alphanumeric characters
            int alphaNumIndices[ssid_old_length];
            int alphaNumCount = 0;

            for (int i = 1; i < ssid_old_length; i++) {
                if (isalnum(ssid[i]))                       // Only alphanumeric chars
                    alphaNumIndices[alphaNumCount++] = i;   // Store the index
            }

            // Shuffle only the alphanumeric characters
            for (int i = 0; i < alphaNumCount; ++i) {
                int j = random(0, alphaNumCount);           // Random index within alphanumeric indices

                // Swap the characters
                char temp = ssid[alphaNumIndices[i]];
                ssid[alphaNumIndices[i]] = ssid[alphaNumIndices[j]];
                ssid[alphaNumIndices[j]] = temp;
            }
        }

        // ----

        // Build the beacon frame
        uint8_t beacon[sizeof(beacon_raw) + 32];  // Max SSID length 32
        memcpy(beacon, beacon_raw, OFFSET_BEACON_SSID - 1);
        beacon[OFFSET_BEACON_SSID - 1] = strlen(ssid);
        memcpy(&beacon[OFFSET_BEACON_SSID], ssid, strlen(ssid));
        memcpy(&beacon[OFFSET_BEACON_SSID + strlen(ssid)], &beacon_raw[OFFSET_BEACON_SSID], sizeof(beacon_raw) - OFFSET_BEACON_SSID);

        // Send 12-bit random sequence number
        uint16_t seq = esp_random() & 0xFFF;
        beacon[OFFSET_BEACON_SEQNUM] = (seq << 4) & 0xF0;
        beacon[OFFSET_BEACON_SEQNUM + 1] = (seq >> 4) & 0xFF;

        esp_wifi_80211_tx(WIFI_IF_AP, beacon, sizeof(beacon_raw) + strlen(ssid), false);
    }
}

// -----------------------------------------------------------------------
// Task process
// -----------------------------------------------------------------------
void UberPayload::payloadTask(void *pvParameter) {
  while (1) {
    if (running) {
        randomizeBeaconSrcMac(beacon_raw);
        //updateBeaconTimestamp(beacon_raw);

        // Run function depending on currently set payload
        switch(payload) {
            case Payload::WIFI_SSID_RICKROLL:
                wifiSsidRickRoll();
                break;
            case Payload::WIFI_SSID_RANDOMALPHA:
                wifiSsidRandomAlpha();
                break;
            case Payload::WIFI_SSID_CLONIE:
            case Payload::WIFI_SSID_CUTIE:
            case Payload::WIFI_SSID_DYXLISEA:
                wifiSsidCloneSpam();
                break;
            default:
                break;
        }

        // Delay each beacon frame iteration
        vTaskDelay(interval / portTICK_PERIOD_MS);

        // Blink LED every 250ms
        millis() % 500 >= 250 ? digitalWrite(LED_PIN, 255) : digitalWrite(LED_PIN, 0);
    } else {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}