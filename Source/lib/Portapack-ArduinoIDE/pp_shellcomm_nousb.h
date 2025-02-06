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
 * https://github.com/htotoo/ESP32-Portapack/blob/main/Source/main/ppshellcomm.h
 *
 * Modifications made by ubermood:
 *   1) Strip USB-communication functionality, since it's not compatible with Arduino IDE
 *      and not needed in this project
 *   2) Changed QUEUE_SIZE from 50 to 1024, since replies from Shell was clipping
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

#ifndef PPSHELLCOMM_H
#define PPSHELLCOMM_H

#define QUEUE_SIZE 1024 // 50

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct
{
    uint8_t size;     // Actual data size (1-64 bytes)
    uint8_t data[64]; // Fixed 64-byte buffer
} I2CQueueMessage_t;

class PPShellComm
{
public:
    static void init();
    static bool write(const uint8_t *data, size_t len);
    static bool write_blocking(const uint8_t *data, size_t len);
    //static bool write(const uint8_t *data, size_t len, bool mute, bool buffer);
    //static bool write_blocking(const uint8_t *data, size_t len, bool mute, bool buffer);
    static bool getInCommand() { return inCommand; }
    static uint8_t getAnyConnected()
    {
        uint8_t ret = 0;
        //if (usb_connected)
        //    ret += 1;
        if (i2c_connected)
            ret += 2;
        return ret;
    }
    static void set_data_rx_callback(bool (*callback)(const uint8_t *data, size_t data_len))
    {
        data_rx_callback = callback;
    }
    static void set_i2c_connected(bool connected)
    {
        i2c_connected = connected;
        inCommand = false;
    }
    static uint16_t get_i2c_tx_queue_size()
    {
        return tx.size;
    }
    static uint8_t *get_i2c_tx_queue_data()
    {
        return tx.data;
    }
    static void clear_tx_queue()
    {
        tx.size = 0;
    }

    static QueueHandle_t datain_queue;

private:
    static void searchPromptAdd(uint8_t ch);
    static void processi2c_queuein_task(void *pvParameters);

    static bool i2c_connected;
    static bool (*data_rx_callback)(const uint8_t *data, size_t data_len);

    static char *PROMPT;
    static char searchPrompt[5];
    static bool inCommand;

    static SemaphoreHandle_t send_block_sem;
    static I2CQueueMessage_t tx;
};

#endif // PPSHELLCOMM_H
