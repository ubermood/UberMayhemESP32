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
 * https://github.com/htotoo/ESP32-Portapack/blob/main/Source/main/ppshellcomm.cpp
 *
 * Modifications made by ubermood:
 *   1) Strip USB-communication functionality, since it was not compatible with Arduino IDE
 *      and not needed in this project
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#include "pp_shellcomm_nousb.h"

#define TAG "PPShellComm"

bool (*PPShellComm::data_rx_callback)(const uint8_t *data, size_t data_len) = nullptr;

bool PPShellComm::i2c_connected = false;
char *PPShellComm::PROMPT = (char *)"ch> ";
char PPShellComm::searchPrompt[5] = {0};
bool PPShellComm::inCommand = false;

SemaphoreHandle_t PPShellComm::send_block_sem;
QueueHandle_t PPShellComm::datain_queue;
I2CQueueMessage_t PPShellComm::tx;

void PPShellComm::init()
{
    datain_queue = xQueueCreate(QUEUE_SIZE, sizeof(I2CQueueMessage_t));

    // Added from usb_init() { .. } method that has been stripped out from this class
    send_block_sem = xSemaphoreCreateBinary();
    assert(send_block_sem);

    // i2c rx handler init
    xTaskCreate(processi2c_queuein_task, "i2crxinth", 4096, xTaskGetCurrentTaskHandle(), 20, NULL);
}

//bool PPShellComm::write(const uint8_t *data, size_t len, bool mute, bool buffer)
bool PPShellComm::write(const uint8_t *data, size_t len)
{
    if (i2c_connected)
    {
        tx.size = len;
        ESP_LOGE(TAG, "i2c write");
        if (len > 64)
        {
            ESP_LOGE(TAG, "Data too big for I2C");
            return false;
        }
        memcpy(tx.data, data, len); // Buffered write
        inCommand = true;
        return true;
    }
    return false;
}

//bool PPShellComm::write_blocking(const uint8_t *data, size_t len, bool mute, bool buffer)
bool PPShellComm::write_blocking(const uint8_t *data, size_t len)
{
    if (i2c_connected)
    {
        tx.size = len;
        if (len > 64)
        {
            ESP_LOGE(TAG, "Data too big for I2C");
            return false;
        }
        memcpy(tx.data, data, len); // Buffered blocking write

        // Block additional writes before PROMPT appears or 5 sec timeout
        //xSemaphoreTake(send_block_sem, 5000 / portTICK_PERIOD_MS) == pdTRUE;
        return true;
    }
    return false;
}

void PPShellComm::processi2c_queuein_task(void *pvParameters)
{
    I2CQueueMessage_t rx;
    while (1)
    {
        if (xQueueReceive(datain_queue, &rx, portMAX_DELAY))
        {
            for (size_t i = 0; i < rx.size; ++i)
            {
                searchPromptAdd(rx.data[i]);
            }

            if (data_rx_callback)
                data_rx_callback(rx.data, rx.size);
        }
    }
}

void PPShellComm::searchPromptAdd(uint8_t ch)
{
    // shift
    for (uint8_t i = 0; i < 3; ++i)
    {
        searchPrompt[i] = searchPrompt[i + 1];
    }
    searchPrompt[3] = ch;
    if (strncmp(PROMPT, searchPrompt, 4) == 0)
    {
        inCommand = false;
        xSemaphoreGive(send_block_sem);
    }
}