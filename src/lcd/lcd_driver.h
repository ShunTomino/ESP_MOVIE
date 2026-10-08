// LCD Driver for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <driver/gpio.h>
#include <driver/ledc.h>

#ifdef ARDUINO // Arduino IDE
    #include "bus/bus_i80.h"
    #include "font/font8x16/font_8x16.h"
#else // ESP_PLATFORM(ESP-IDF)
    #include "bus_i80.h"
    #include "font_8x16.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


// RGB565 Color
#define RGB565_BLACK 0x0000
#define RGB565_WHITE 0xFFFF
#define RGB565_RED   0xF800
#define RGB565_GREEN 0x07E0
#define RGB565_BLUE  0x001F
#define RGB565_YELLOW  0xFFE0
#define RGB565_MAGENTA 0xF81F
#define RGB565_CYAN    0x07FF

#define LCD_COLOR_TABLE_ENABLE 1

#if LCD_COLOR_TABLE_ENABLE
    // Color adjustment for color table
    #define GAMMA_CURVE 1 // 0:Gamma 4.0, 1:Gamma 3.0, 2:Gamma 2.0, 3:Gamma 1.0
    #define COLOR_VOL_MAX 55 // Maximum color volume (Max:63)
    #define COLOR_VOL_OFFSET 0 // Color volume offset (0~63)
#endif


#define PIXEL_TX_CMD 0x2C // LCD Memory Write Command


// LCD Back Light PWM Initialization
esp_err_t init_backlight();

// LCD Back LIght Volume Control : 0〜255
esp_err_t backlight_volume(uint8_t volume);

// Set the window area
// you don't need to send pixel data outside the specified range.
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// Set the window area for displaying an image
// x0, y0 are offset value, but you set the value to negative, the image is centered.
// <return> 0:OK, -1:Over Width Error, -2:Over Height Error
int8_t set_image_window(uint16_t image_width, uint16_t image_height, int16_t x0, int16_t y0);

// Set the X/Y axis of the display
// <x_inverse> 0:Normal, 1:Inverse 
// <y_inverse> 0:Normal, 1:Inverse
// <xy_exchange> 0:Normal, 1:Exchange
void set_axis(bool x_inverse, bool y_inverse, bool xy_exchange);

// Display Initialization
esp_err_t init_display();

#if !JPG_BLOCK_DEC_ENABLE
// Sending all of the image buffer data by DMA
void send_image(size_t size);

// Set a specific color in a rectangular area of the image buffer
// Not sent to the LCD yet. Use send_image() to send it to the LCD.
void set_color(uint16_t rgb565, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
#endif

// Fill the entire image buffer with a specific color.
// Not sent to the LCD yet. 
void set_color_all(uint16_t rgb565);

// Display a specific color on the entire LCD screen
void display_color_all(uint16_t rgb565);

// Display a specific color in a rectangular area of the LCD screen
void display_color(uint16_t rgb565, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// For test sending without DMA
void display_color_all_NoDMA(uint16_t rgb565);

#if !JPG_BLOCK_DEC_ENABLE
// Set 1 line string for LCD buffer
int8_t set_string(const char *str, uint16_t rgb565, uint16_t lcd_x, uint16_t lcd_y);
#endif

// Send 1 line string to LCD
// <return> 0:OK, minus value: Sting length is over the display window size
int8_t display_string(const char *str, uint16_t rgb565_font, uint16_t rgb565_back, uint16_t x0, uint16_t y0);


#ifdef __cplusplus
}
#endif
