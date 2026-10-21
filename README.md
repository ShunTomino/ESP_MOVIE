# ESP_MOVIE
Movie (Motion JPEG and MP3) playback library for ESP32 series

[![Arduino Library](https://www.ardu-badge.com/badge/ESP_MOVIE.svg)](https://www.ardu-badge.com/ESP_MOVIE)
[![ESP Component Registry](https://components.espressif.com/components/shuntomino/esp_movie/badge.svg)](https://components.espressif.com/components/shuntomino/esp_movie)
[![GitHub Release](https://img.shields.io/github/v/release/ShunTomino/ESP_MOVIE)](https://github.com/ShunTomino/ESP_MOVIE/releases)
 
## Features
- Simultaneous playback of Motion JPEG video and MP3 audio from SD card files.
- Motion JPEG-only playback.
- MP3-only playback.
- JPEG image display.
- Text rendering.
- Rectangle fill operation for a specified display area.
- Display backlight brightness control.

## Platform
- Arduino IDE (arduino-esp32)
- ESP-IDF

## Hardware Support
### MCU
- Currently tested on ESP32S3 only. Support for other ESP32 series devices has not been verified yet.

### Display
- Supports Intel 8080 (8-bit parallel) interface with DMA transfer. SPI interface is not supported.
- Supports RGB565/BGR565 color format.
- Currently tested with the following display controllers. 
  - ILI9342
  - ST7789P3
  
  Other MIPI DCS-compatible display controllers may also work, but have not been tested.

### Audio
- Supports I2S audio output.
- Tested with the PCM5102 DAC. 

### SD Card
- Supports SPI and SDMMC(1-Line) modes.


## Included Sample Files
The examples include sample Motion JPEG (AVI), MP3, JPEG files for quick testing.


## Included Libraries
ESP_MOVIE uses the following libraries.
- [esp-libhelix-mp3](https://github.com/chmorgan/esp-libhelix-mp3.git) (chmorgan): Apache License 2.0
- [esp_new_jpeg](https://github.com/espressif/esp-adf-libs/tree/master/esp_new_jpeg) (espressif): Espressif MIT License
- [font8x16](https://github.com/hubenchang0515/font8x16.git) (hubenchang0515): MIT License

##
<img width="786" height="800" alt="662273982-568e9c63-7523-4765-9915-dac9b773f11e" src="https://github.com/user-attachments/assets/8bd22a78-9cd4-4ce4-82de-2fce5628feb0" />
<img width="1728" height="860" alt="662274187-f6c32763-96d2-4800-821c-29fa1cdfe6c9" src="https://github.com/user-attachments/assets/560dd7c4-e7a4-4b15-bf7a-436755e216a4" />

