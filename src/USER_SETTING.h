// User Setting for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/i2s_std.h>
#include <esp_vfs_fat.h>


/*---------- SD CARD Setting ----------*/
// SPI Pin Assign
#define PIN_SD_SPI_MOSI GPIO_NUM_21
#define PIN_SD_SPI_MISO GPIO_NUM_13
#define PIN_SD_SPI_SCK  GPIO_NUM_14
#define PIN_SD_SPI_CS   GPIO_NUM_NC

// SPI Setting
#define SD_SPI_FREQ_kHz 20000 // Max:2000kHz
#define SD_SPI_HOST SPI2_HOST

// File setting
#define SD_MAX_FILES 3 // Min:3
#define MAX_PATH_LEN 64
#define PATH_SD "/sd"


/*---------- Display Setting ----------*/
// Intel8080 Bus(8 Bit Parallel Bus) Pin Assign
#define PIN_LCD_D0 GPIO_NUM_16
#define PIN_LCD_D1 GPIO_NUM_17
#define PIN_LCD_D2 GPIO_NUM_18
#define PIN_LCD_D3 GPIO_NUM_8
#define PIN_LCD_D4 GPIO_NUM_9
#define PIN_LCD_D5 GPIO_NUM_10
#define PIN_LCD_D6 GPIO_NUM_11
#define PIN_LCD_D7 GPIO_NUM_12
#define PIN_LCD_DC GPIO_NUM_15
#define PIN_LCD_WR GPIO_NUM_7
#define PIN_LCD_RD GPIO_NUM_6
#define PIN_LCD_CS GPIO_NUM_NC // Chip Select Pin, if you don't use this pin, set GPIO_NUM_NC.

#define PIN_LCD_RST GPIO_NUM_5 // Reset Pin
#define PIN_LCD_BL  GPIO_NUM_4 // BackLight Pin

// Intel8080 Bus Clock
#define LCD_CLK_FREQ 20000000 // Max:20000000Hz

// LCD Pixcel Width/Height 
#define LCD_WIDTH  320
#define LCD_HEIGHT 240

#define LCD_XY_EXCHANGE 1 // 0:Normal, 1:Exchange
#define LCD_X_INVERSE 1 // 0:Normal, 1:Inverse
#define LCD_Y_INVERSE 0 // 0:Normal, 1:Inverse

#define LCD_RGB_FORMAT 0 // 0:RGB, 1:BGR
#define LCD_DATA_INVERSE 1 // 0:Normal, 1:Inverse


/*---------- JPG/MJPG Setting ----------*/
// Block Decode Mode(Minimal RAM use), if PSRAM is supported, you can set 0.
// When set 1, the JPG/MJPG is decoded in block unit, the image width/height must be times of 16.
#define JPG_BLOCK_DEC_ENABLE 1

// Buffer for reading JPG file or MotionJPG Frame data from SD card.
// if JPG_BLOCK_DEC_ENABLE == 1, JPG_BUF is automatically placed to PSRAM.
#define JPG_BUF_SIZE 40000 // Min: JPG file size or MotionJPG Frame data size
#define JPG_BUF_ENABLE_PSRAM 0 // if PSRAM is supported, you can set 0.

// LCD Back Light PWM Control Setting
#define LEDC_PWM_CHANNEL LEDC_CHANNEL_0
#define LEDC_PWM_TIMER_NUM LEDC_TIMER_0


/*---------- MP3 Setting ----------*/
// I2S DAC Pin Assign
#define PIN_I2S_BCLK   GPIO_NUM_37 // to BCK
#define PIN_I2S_DOUT   GPIO_NUM_36 // to DIN
#define PIN_I2S_LRC    GPIO_NUM_35 // to LRCK(WS)
#define PIN_AUDIO_MUTE GPIO_NUM_48 // High:unmute, Low:mute, if you don't use this pin, set GPIO_NUM_NC.

#define I2S_PORT_NUM I2S_NUM_0

// Buffer for reading MP3 file from SD card
#define MP3_BUF_SIZE 1500 // Default 1500 Byte
#define MP3_BUF_ENABLE_PSRAM 0  // if PSRAM is supported, you can set 0.
