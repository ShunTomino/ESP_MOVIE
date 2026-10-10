// JPG/MJPG Decoder for ESP_MOVIE library
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

#include <esp_timer.h>

#ifdef ARDUINO // Arduino IDE
    #include "sdcard/sd_spi.h"
    #include "lcd/lcd_driver.h"
	#include "esp_new_jpeg/src/esp_jpeg_dec.h"
#else // ESP_PLATFORM(ESP-IDF)
    #include "sd_spi.h"
    #include "lcd_driver.h"
	#include "esp_jpeg_dec.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


// MJPG display time offset(microsec per frame)
#define MJPG_DELAY_PER_FRAME (-2)

typedef enum {
    JPG_OK = JPEG_ERR_OK,
    JPG_ERR_FAIL = JPEG_ERR_FAIL,
    JPG_ERR_NO_MEM = JPEG_ERR_NO_MEM,
    JPG_ERR_NO_MORE_DATA = JPEG_ERR_NO_MORE_DATA,
    JPG_ERR_INVALID_PARAM = JPEG_ERR_INVALID_PARAM,
    JPG_ERR_BAD_DATA = JPEG_ERR_BAD_DATA,
    JPG_ERR_UNSUPPORT_FMT = JPEG_ERR_UNSUPPORT_FMT,
    JPG_ERR_UNSUPPORT_STD = JPEG_ERR_UNSUPPORT_STD,

    JPG_ERR_OPEN_FILE = -8,
    JPG_ERR_OPEN_LOADFILE = -9,
    JPG_ERR_IMAGE_SIZE_OVER = -10,
    JPG_ERR_RESOLUTION_UNMATCH = -11,
    JPG_ERR_END_COMMAND = -12,
    JPG_ERR_INIT_FAIL = -13,
    JPG_ERR_DEINIT_FAIL = -14,
} jpg_err_t;


typedef struct {
    uint32_t dwMicroSecPerFrame;
    uint32_t dwTotalFrames;
    uint32_t dwWidth;
    uint32_t dwHeight;
} avi_info_t;


typedef struct {
    #if JPG_BLOCK_DEC_ENABLE
        int block_size;
        int block_count;
        int remain_size;
    #else
        size_t output_size;
    #endif

    char *path_mjpg;
    char *path_mjpgload;
    int64_t decoded_time; // Time from start to end of MJPG (microsec)
    uint32_t total_frames;
    uint32_t microsec_per_frame;
} video_info_t;


extern uint8_t jpg_buf[JPG_BUF_SIZE];

extern char path_mjpgload[MAX_PATH_LEN];

extern SemaphoreHandle_t mjpg_dec_standby; // Semaphore for decode standby
extern SemaphoreHandle_t mjpg_dec_start; // Semaphore for decode start
extern SemaphoreHandle_t mjpg_dec_end; // Semaphore for decode end

extern bool mjpgContinue; // Continue command
extern bool mjpgPause; // Pause command


// MJPGバックグラウンド再生終了時のコールバック関数
typedef void (*FuncEndMJPG)(video_info_t*);


jpg_err_t init_jpg_decoder(FuncEndMJPG func_end);


jpg_err_t deinit_jpg_decoder();


// Display JPG image
// x0, y0 are offset value for the window, if you set the value to negatve, the image is centerd.
// jpg_info is set by this function
jpg_err_t display_jpg(char *path_jpg, int16_t x0, int16_t y0, jpeg_dec_header_info_t *jpg_info);


// Set path of directory for mjpgload file
void set_dir_for_mjpgload(char *path_mjpgload, const char *path_directory);


// Set path for mjpgload file to path_mjpgload
int8_t get_path_mjpgload(char *path_mjpgload, const char *path_mjpg);


// Get AVI file information
// OK: return 0, NG: return minus value below.
// -1: RIFF Container format NG
// -2: AVI format NG
// -3: VIDEO type NG
// -4: MJPG Compression format NG
int8_t get_avi_info(avi_info_t *avi_info, FILE *mjpg_file);


uint32_t search_mjpg_frame(const char *path_mjpg);


void mjpg_start_command();


void mjpg_pause_command();


void mjpg_end_command();


void task_mjpg_decode(void *video_info);


// x0, y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t set_mjpg(char *path_mjpg, int16_t x0, int16_t y0, const BaseType_t Core0or1, UBaseType_t taskPriority);


// x0, y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t start_mjpg(char *path_mjpg, int16_t x0, int16_t y0, const BaseType_t Core0or1, UBaseType_t taskPriority);


// x0, y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t play_mjpg(char *path_mjpg, int16_t x0, int16_t y0);

#ifdef __cplusplus
}
#endif
