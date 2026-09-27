// Intel 8080 bus (8bit parallel bus) for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <esp_lcd_panel_io.h>
#include <driver/gpio.h>
//#include <esp_log.h>

#include "USER_SETTING.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_DMA_TRANSFER_SIZE (4095*25) // ESP-IDF v6.1
//#define MAX_DMA_TRANSFER_SIZE (4095*26) // ESP-IDF v5.5.5

#if JPG_BLOCK_DEC_ENABLE
    #define IMGBUF_COUNT 2 // Double buffer
    #define IMGBUF_LEN (LCD_WIDTH * 16) // Max image block length
    extern uint16_t imgBuf[IMGBUF_COUNT][IMGBUF_LEN];

    extern uint32_t IMGBUF_LEN_MAX;
    extern size_t IMGBUF_SIZE_MAX;
    extern volatile bool imgBuf_busy[IMGBUF_COUNT];
#else
    #define IMGBUF_COUNT 1
    #define IMGBUF_LEN (LCD_WIDTH*LCD_HEIGHT) // Full frame length 
    extern uint16_t imgBuf[IMGBUF_LEN];

    extern uint32_t IMGBUF_LEN_MAX;
    extern size_t IMGBUF_SIZE_MAX;
    extern volatile bool imgBuf_busy;

    #if (IMGBUF_LEN * 2 > MAX_DMA_TRANSFER_SIZE)
        extern size_t max_transfer_size;
    #endif
#endif

extern uint32_t IMG_LEN;
extern size_t IMG_SIZE; // RGB565:2byte/pixel

extern esp_lcd_i80_bus_handle_t i80_bus;
extern esp_lcd_panel_io_handle_t i80_handle;


// 8bit Command Sending
void send_command(uint8_t cmd);


// 8bit Data Sending
void send_data(uint8_t data);


#if JPG_BLOCK_DEC_ENABLE
// Set double DMA buffer for decoded image block
void set_double_image_buffer(uint16_t **img_buf, volatile bool **img_buf_busy);
#endif


// Intel8080 8bit pararel bus Initialization
esp_err_t init_i80();


esp_err_t deinit_i80();


#ifdef __cplusplus
}
#endif