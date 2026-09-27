// MP3 Decoder for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <driver/i2s_std.h>
#include <driver/i2s_pdm.h>
#include <driver/gpio.h>

#ifdef ARDUINO // Arduino IDE
	#include "sdcard/sd_spi.h"
	#include "esp-libhelix-mp3/mp3dec.h"
#else // ESP_PLATFORM(ESP-IDF)
	#include "sd_spi.h"
	#include "mp3dec.h"
#endif

#include "USER_SETTING.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_VOLUME 20

#if MP3_BUF_SIZE < 1445
	#define MP3_BUF_SIZE 1445
#endif

typedef enum {
	MP3_OK = ERR_MP3_NONE,
	MP3_ERR_INDATA_UNDERFLOW = ERR_MP3_INDATA_UNDERFLOW,
	MP3_ERR_MAINDATA_UNDERFLOW = ERR_MP3_MAINDATA_UNDERFLOW,
	MP3_ERR_FREE_BITRATE_SYNC = ERR_MP3_FREE_BITRATE_SYNC,
	MP3_ERR_OUT_OF_MEMORY = ERR_MP3_OUT_OF_MEMORY,
	MP3_ERR_NULL_POINTER = ERR_MP3_NULL_POINTER,
	MP3_ERR_INVALID_FRAMEHEADER = ERR_MP3_INVALID_FRAMEHEADER,
	MP3_ERR_INVALID_SIDEINFO = ERR_MP3_INVALID_SIDEINFO,
	MP3_ERR_INVALID_SCALEFACT = ERR_MP3_INVALID_SCALEFACT,
	MP3_ERR_INVALID_HUFFCODES = ERR_MP3_INVALID_HUFFCODES,
	MP3_ERR_INVALID_DEQUANTIZE = ERR_MP3_INVALID_DEQUANTIZE,
	MP3_ERR_INVALID_IMDCT = ERR_MP3_INVALID_IMDCT,
	MP3_ERR_INVALID_SUBBAND = ERR_MP3_INVALID_SUBBAND,
	
    MP3_ERR_OPEN_FILE = -13,
	MP3_ERR_END_COMMAND = -14,
	
	MP3_ERR_UNKNOWN = ERR_UNKNOWN,
} mp3_err_t;


typedef struct {
	char *path_mp3;
    uint16_t output_size;
} audio_info_t;

// Buffer for MP3 frame data read
extern uint8_t mp3Buf[MP3_BUF_SIZE];

extern SemaphoreHandle_t mp3_dec_standby; // Semaphore for decode standby
extern SemaphoreHandle_t mp3_dec_start; // Semaphore for decode start
extern SemaphoreHandle_t mp3_dec_end; // Semaphore for decode end

extern bool mp3Continue; // Continue command
extern bool mp3Pause; // Pause command


// MP3バックグラウンド再生終了時のコールバック関数
typedef void (*FuncEndMP3)(audio_info_t*);


// I2S DAC Initialization
esp_err_t init_i2s_dac();


// I2S DAC Deinitialization
esp_err_t deinit_i2s_dac();


// Change Output Sample Rate of I2S DAC
esp_err_t change_audio_output_sampleRate(int audio_samplerate);


void set_audio_volume(int16_t volume);


void audio_silence();


#if (PIN_AUDIO_MUTE != -1)
	void audio_mute();
	void audio_unmute();
#endif


void init_mp3_decoder(FuncEndMP3 func_end);


void deinit_mp3_decoder();


void mp3_start_command();


void mp3_pause_command();


void mp3_end_command();


mp3_err_t set_mp3(char *path_mp3, const BaseType_t Core0or1, UBaseType_t taskPriority);


mp3_err_t start_mp3(char *path_mp3, const BaseType_t Core0or1, UBaseType_t taskPriority);


mp3_err_t play_mp3(char *path_mp3);

#ifdef __cplusplus
}
#endif
