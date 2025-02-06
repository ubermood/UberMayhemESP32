/*
 * Copyright (C) 2024 HTotoo
 *
 * This file is part of ESP32-Portapack.
 * 
 * For additional license information, see the LICENSE file.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
 * NOTE!
 * This file has been modified from the original version.
 *
 * The original code can be found at:
 * https://github.com/htotoo/ESP32-Portapack/blob/main/Source/main/ppi2c/pp_handler.cpp
 *
 * Modifications made by ubermood:
 *   1) Use 'Wire.h' Arduino library instead of 'i2c_slave_driver.h', that was originally used
 *   2) Add I2C frequency as an argument to init(), since Wire uses 100kHz as default frequency (PP uses 400kHz)
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include "pp_handler.h"

uint8_t PPHandler::addr = 0;
volatile Command PPHandler::command_state = Command::COMMAND_NONE;
volatile uint16_t PPHandler::app_counter = 0;
volatile uint16_t PPHandler::app_transfer_block = 0;

get_features_CB PPHandler::features_cb = nullptr;
get_gps_data_CB PPHandler::gps_data_cb = nullptr;
get_orientation_data_CB PPHandler::orientation_data_cb = nullptr;
get_environment_data_CB PPHandler::environment_data_cb = nullptr;
get_light_data_CB PPHandler::light_data_cb = nullptr;

get_shell_data_size_CB PPHandler::shell_data_size_cb = nullptr;
got_shell_data_CB PPHandler::got_shell_data_cb = nullptr;
send_shell_data_CB PPHandler::send_shell_data_cb = nullptr;

std::vector<app_list_element_t> PPHandler::app_list;
std::vector<pp_custom_command_list_element_t> PPHandler::custom_command_list;
uint32_t PPHandler::module_version = 1;
char PPHandler::module_name[20] = "ESP32MODULE";

void PPHandler::init(int sda, int scl, uint8_t addr_, uint32_t freq) {
    addr = addr_;

    Wire.begin(addr, sda, scl, freq);  // I2C slave mode
    Wire.onRequest(handleI2CRequest);
    Wire.onReceive(handleI2CReceive);
}

uint32_t PPHandler::get_appCount() {
    return (uint32_t)app_list.size();
}

// region Callback setters

void PPHandler::set_get_features_CB(get_features_CB cb) {
    features_cb = cb;
}

void PPHandler::set_get_gps_data_CB(get_gps_data_CB cb) {
    gps_data_cb = cb;
}

void PPHandler::set_get_orientation_data_CB(get_orientation_data_CB cb) {
    orientation_data_cb = cb;
}

void PPHandler::set_get_environment_data_CB(get_environment_data_CB cb) {
    environment_data_cb = cb;
}

void PPHandler::set_get_light_data_CB(get_light_data_CB cb) {
    light_data_cb = cb;
}

void PPHandler::set_get_shell_data_size_CB(get_shell_data_size_CB cb) {
    shell_data_size_cb = cb;
}

void PPHandler::set_got_shell_data_CB(got_shell_data_CB cb) {
    got_shell_data_cb = cb;
}

void PPHandler::set_send_shell_data_CB(send_shell_data_CB cb) {
    send_shell_data_cb = cb;
}

// endregion

void PPHandler::set_module_name(std::string name) {
    strncpy(module_name, name.c_str(), 20);
    module_name[19] = 0;
}

void PPHandler::set_module_version(uint32_t version) {
    module_version = version;
}

bool PPHandler::add_app(uint8_t* binary, uint32_t size) {
    if (size % 32 != 0 || size < sizeof(standalone_app_info)) {
        esp_rom_printf("FAILED ADDING APP, BAD SIZE\n");
        return false;
    }

    app_list.push_back({binary, size});
    return true;
}

void PPHandler::add_custom_command(uint16_t command, pp_i2c_command got_command, pp_i2c_command send_command) {
    pp_custom_command_list_element_t element;
    element.command = command;
    element.got_command = got_command;
    element.send_command = send_command;
    custom_command_list.push_back(element);
}

void PPHandler::on_command_ISR(Command command, std::vector<uint8_t> additional_data) {
    command_state = command;

    switch (command) {
        case Command::COMMAND_APP_INFO:
            if (additional_data.size() == 2)
                app_counter = *(uint16_t*)additional_data.data();
            break;

        case Command::COMMAND_APP_TRANSFER:
            if (additional_data.size() == 4) {
                app_counter = *(uint16_t*)additional_data.data();
                app_transfer_block = *(uint16_t*)(additional_data.data() + 2);
            }
            break;

        case Command::COMMAND_GETFEATURE_MASK:
            break;

        case Command::COMMAND_SHELL_PPTOMOD_DATA:
            if (got_shell_data_cb)
                got_shell_data_cb(additional_data);
            break;

        default:
            for (auto element : custom_command_list) {
                if (element.command == (uint16_t)command) {
                    if (element.got_command) {
                        pp_command_data_t data;
                        data.data = &additional_data;
                        element.got_command(data);
                    }
                    break;
                }
            }
            break;
    }
    //command_state = Command::COMMAND_NONE;
}

// 1 - Data in (master sends command + additional data to slave)
void PPHandler::handleI2CReceive(int numBytes) {
    // Read all data

    //std::vector<uint8_t> data_in;
    //while (Wire.available())
    //    data_in.push_back(Wire.read());

    std::vector<uint8_t> data_in(numBytes);
    Wire.readBytes(data_in.data(), numBytes);

    uint16_t command;
    if (data_in.size() >= 2) {
        // 2 first bytes of data is the command
        command = (data_in[1] << 8) | data_in[0];
        command_state = (Command)command;

        // Rest of the bytes are additional data
        if (command_state != Command::COMMAND_NONE && data_in.size() > 2) {
            // Remove command from additional data
            std::vector<decltype(data_in)::value_type>(data_in.begin()+2, data_in.end()).swap(data_in);

            // Handle additional data
            on_command_ISR((Command)command, data_in);
        }
    }

    //Serial.printf("%08ld [I] cmd:%02d, len:%d, data_len:%d\r\n", millis(), command_state, numBytes, data_in.size());
}

// 2 - Data out (slave responds with data to master)
void PPHandler::handleI2CRequest() {
    auto data_out = on_send_ISR();

    if (data_out.size() > 0)
        Wire.write(data_out.data(), data_out.size());
        //Wire.slaveWrite(data_out.data(), data_out.size());

    //Serial.printf("%08ld [O] cmd:%02d, len:%d\r\n", millis(), command_state, data_out.size());
}

/*
void printHexData(const std::vector<uint8_t>& data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        Serial.printf("%02X, ", data[i]);
    }
    Serial.printf("\r\n");

    for (size_t i = 0; i < size; i++) {
        Serial.printf("%c", data[i]);
    }
    Serial.printf("\r\n");
}
*/

// this handle, when the PP needs data
std::vector<uint8_t> PPHandler::on_send_ISR() {
    switch (command_state) {
        case Command::COMMAND_INFO: {
            device_info info = {
                PP_API_VERSION,     // api_version
                module_version,     // module_version
                "",                 // module_name
                app_list.size()     // application_count
            };
            strncpy(info.module_name, module_name, 20);

            return std::vector<uint8_t>((uint8_t*)&info, (uint8_t*)&info + sizeof(info));
        }

        case Command::COMMAND_APP_INFO: {
            if (app_counter <= app_list.size() - 1) {
                standalone_app_info app_info;
                std::memset(&app_info, 0, sizeof(app_info));
                std::memcpy(&app_info, app_list[app_counter].binary, sizeof(app_info) - 4);
                app_info.binary_size = app_list[app_counter].size;
                app_counter = app_counter + 1;
                return std::vector<uint8_t>((uint8_t*)&app_info, (uint8_t*)&app_info + sizeof(app_info));
            }
            break;
        }

        case Command::COMMAND_APP_TRANSFER: {
            if (app_counter <= app_list.size() - 1 && app_transfer_block < app_list[app_counter].size / 128) {
                return std::vector<uint8_t>(app_list[app_counter].binary + app_transfer_block * 128, app_list[app_counter].binary + app_transfer_block * 128 + 128);
            }
            break;
        }

        case Command::COMMAND_GETFEATURE_MASK: {
            uint64_t features = 0;
            if (features_cb)
                features_cb(features);
            else
                features = app_list.size() > 0 ? (uint64_t)SupportedFeatures::FEAT_EXT_APP : (uint64_t)SupportedFeatures::FEAT_NONE;  // default, only check if ext app added or not

            return std::vector<uint8_t>((uint8_t*)&features, (uint8_t*)&features + sizeof(features));
        }

        case Command::COMMAND_GETFEAT_DATA_GPS: {
            ppgpssmall_t gpsdata = {};
            if (gps_data_cb)
                gps_data_cb(gpsdata);

            return std::vector<uint8_t>((uint8_t*)&gpsdata, (uint8_t*)&gpsdata + sizeof(gpsdata));
        }

        case Command::COMMAND_GETFEAT_DATA_ORIENTATION: {
            orientation_t ori = {400, 400};  // false data
            if (orientation_data_cb)
                orientation_data_cb(ori);

            return std::vector<uint8_t>((uint8_t*)&ori, (uint8_t*)&ori + sizeof(ori));
        }

        case Command::COMMAND_GETFEAT_DATA_ENVIRONMENT: {
            environment_t env = {};
            if (environment_data_cb)
                environment_data_cb(env);

            return std::vector<uint8_t>((uint8_t*)&env, (uint8_t*)&env + sizeof(env));
        }

        case Command::COMMAND_GETFEAT_DATA_LIGHT: {
            uint16_t light = 0;
            if (light_data_cb)
                light_data_cb(light);

            return std::vector<uint8_t>((uint8_t*)&light, (uint8_t*)&light + sizeof(light));
        }

        case Command::COMMAND_SHELL_MODTOPP_DATA_SIZE: {
            uint16_t size = 0;
            if (shell_data_size_cb)
                size = shell_data_size_cb();

            return std::vector<uint8_t>((uint8_t*)&size, (uint8_t*)&size + sizeof(size));
        }

        case Command::COMMAND_SHELL_MODTOPP_DATA: {
            std::vector<uint8_t> data;
            if (send_shell_data_cb) {
                bool hasmore = false;
                send_shell_data_cb(data, hasmore);
                uint8_t pre = hasmore ? 0x80 : 0x00;
                pre |= data.size();
                data.insert(data.begin(), pre);
                data.resize(65);
            }
            if (data.size() == 0) {
                return {0xFF};
            }

            return data;
        }

        default: {
            for (auto element : custom_command_list) {
                if (element.command == (uint16_t)command_state) {
                    if (element.send_command) {
                        std::vector<uint8_t> tmp;
                        pp_command_data_t data = {&tmp};
                        element.send_command(data);
                        return tmp;
                    }
                    break;
                }
            }
            break;
        }
    }

    return {0xFF};
}

