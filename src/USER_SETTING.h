// User Setting for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/i2s_std.h>
#include <esp_vfs_fat.h>


/*---------- SD CARD Setting ----------*/
// SPI Pin Assign
#define PIN_SD_SPI_MOSI 21
#define PIN_SD_SPI_MISO 13
#define PIN_SD_SPI_SCK  14
#define PIN_SD_SPI_CS   -1 // Chip Select Pin, if you don't use this pin, -1.

// SPI Setting
#define SD_SPI_FREQ_kHz SDMMC_FREQ_DEFAULT // Max:20000kHz
#define SD_SPI_HOST 1 // SPI2_HOST: 1, SPI3_HOST: 2

// File setting
#define SD_MAX_FILES 3 // Min:3
#define MAX_PATH_LEN 64
#define PATH_SD "/sd"


/*---------- Display Setting ----------*/
// Intel8080 Bus(8 Bit Parallel Bus) Pin Assign
#define PIN_LCD_D0 16
#define PIN_LCD_D1 17
#define PIN_LCD_D2 18
#define PIN_LCD_D3 8
#define PIN_LCD_D4 9
#define PIN_LCD_D5 10
#define PIN_LCD_D6 11
#define PIN_LCD_D7 12
#define PIN_LCD_DC 15
#define PIN_LCD_WR 7
#define PIN_LCD_RD 6
#define PIN_LCD_CS -1 // Chip Select Pin, if you don't use this pin, set -1.

#define PIN_LCD_RST 5 // Reset Pin
#define PIN_LCD_BL  4 // BackLight Pin

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

// Enable RGB565->RGB666 Color depth convertion in LCD
// If your LCD supports color depth conversion table, you can set 1.
#define LCD_COLOR_TABLE_ENABLE 0

#if LCD_COLOR_TABLE_ENABLE
    // Color adjustment for color table
    #define GAMMA_CURVE 1 // 0:Gamma 4.0, 1:Gamma 3.0, 2:Gamma 2.0, 3:Gamma 1.0
    #define COLOR_VOL_MAX 55 // Maximum color volume (Max:63)
    #define COLOR_VOL_OFFSET 0 // Color volume offset (0~63)
#endif

/*---------- JPG/MJPG Setting ----------*/
// Block Decode Mode(Minimal RAM use), if PSRAM is supported, you can set 0.
// When set 1, the JPG/MJPG is decoded in block unit, the image width/height must be times of 16.
#define JPG_BLOCK_DEC_ENABLE 1

// Buffer for reading JPG file or MotionJPG Frame data from SD card.
// if JPG_BLOCK_DEC_ENABLE == 1, JPG_BUF is automatically placed to PSRAM.
#define JPG_BUF_SIZE 40000 // Min: JPG file size or MotionJPG Frame data size
#define JPG_BUF_ENABLE_PSRAM 0 // if PSRAM is supported, you can set 0.

// LCD Back Light PWM Control Setting
#define LEDC_PWM_CHANNEL 0 // Ledc Channel 0 ~ 7
#define LEDC_PWM_TIMER_NUM 0 // Ledc Timer 0 ~ 3


/*---------- MP3 Setting ----------*/
// I2S DAC Pin Assign
#define PIN_I2S_BCLK   37 // to BCK
#define PIN_I2S_DOUT   36 // to DIN
#define PIN_I2S_LRC    35 // to LRCK(WS)
#define PIN_AUDIO_MUTE 48 // High:unmute, Low:mute, if you don't use this pin, set -1.

#define I2S_PORT_NUM I2S_NUM_0

// Buffer for reading MP3 file from SD card
#define MP3_BUF_SIZE 1500 // Default 1500 Byte
#define MP3_BUF_ENABLE_PSRAM 0  // if PSRAM is supported, you can set 0.
