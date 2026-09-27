// ESP_MOVIE - Motion JPEG and MP3 player library for ESP32 Series
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once

#ifdef ARDUINO // Arduino IDE
    #include "video/JPG_decoder.h"
    #include "audio/MP3_decoder.h"
#else // ESP_PLATFORM(ESP-IDF)
    #include "JPG_decoder.h"
    #include "MP3_decoder.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


// Movie Error Code
typedef enum {
    MOVIE_OK = 0, // Succeeded

    MOVIE_MJPG_FAIL = JPG_ERR_FAIL,
    MOVIE_MJPG_NO_MEM = JPG_ERR_NO_MEM,
    MOVIE_MJPG_NO_MORE_DATA = JPG_ERR_NO_MORE_DATA,
    MOVIE_MJPG_INVALID_PARAM = JPG_ERR_INVALID_PARAM,
    MOVIE_MJPG_BAD_DATA = JPG_ERR_BAD_DATA,
    MOVIE_MJPG_UNSUPPORT_FMT = JPG_ERR_UNSUPPORT_FMT,
    MOVIE_MJPG_UNSUPPORT_STD = JPG_ERR_UNSUPPORT_STD,
    MOVIE_MJPG_OPEN_FILE = JPG_ERR_OPEN_FILE,
    MOVIE_MJPG_OPEN_LOADFILE = JPG_ERR_OPEN_LOADFILE,
    MOVIE_MJPG_IMAGE_SIZE_OVER = JPG_ERR_IMAGE_SIZE_OVER,
    MOVIE_MJPG_RESOLUTION_UNMATCH = JPG_ERR_RESOLUTION_UNMATCH,
    MOVIE_MJPG_END_COMMAND = JPG_ERR_END_COMMAND,
    MOVIE_MJPG_INIT_FAIL = JPG_ERR_INIT_FAIL,
    MOVIE_MJPG_DEINIT_FAIL = JPG_ERR_DEINIT_FAIL,

    MP3_ERR_OFFSET = MOVIE_MJPG_DEINIT_FAIL,

	MOVIE_MP3_INDATA_UNDERFLOW = (MP3_ERR_INDATA_UNDERFLOW + MP3_ERR_OFFSET),
	MOVIE_MP3_MAINDATA_UNDERFLOW = (MP3_ERR_MAINDATA_UNDERFLOW + MP3_ERR_OFFSET),
	MOVIE_MP3_FREE_BITRATE_SYNC = (MP3_ERR_FREE_BITRATE_SYNC + MP3_ERR_OFFSET),
	MOVIE_MP3_OUT_OF_MEMORY = (MP3_ERR_OUT_OF_MEMORY + MP3_ERR_OFFSET),
	MOVIE_MP3_NULL_POINTER = (MP3_ERR_NULL_POINTER + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_FRAMEHEADER = (MP3_ERR_INVALID_FRAMEHEADER + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_SIDEINFO = (MP3_ERR_INVALID_SIDEINFO + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_SCALEFACT = (MP3_ERR_INVALID_SCALEFACT + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_HUFFCODES = (MP3_ERR_INVALID_HUFFCODES + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_DEQUANTIZE = (MP3_ERR_INVALID_DEQUANTIZE + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_IMDCT = (MP3_ERR_INVALID_IMDCT + MP3_ERR_OFFSET),
	MOVIE_MP3_INVALID_SUBBAND = (MP3_ERR_INVALID_SUBBAND + MP3_ERR_OFFSET),
    MOVIE_MP3_OPEN_FILE = (MP3_ERR_OPEN_FILE + MP3_ERR_OFFSET),
    MOVIE_MP3_END_COMMAND = (MP3_ERR_END_COMMAND + MP3_ERR_OFFSET),

	MOVIE_MP3_UNKNOWN = (MP3_ERR_UNKNOWN + MP3_ERR_OFFSET),
    
    MOVIE_END_COMMAND = -100,
} movie_err_t;


movie_err_t init_movie_player(FuncEndMJPG func_end_MJPG, FuncEndMP3 func_end_MP3);


movie_err_t deinit_movie_player();


// Set path of directory for mp3load file and mjpg load file
void set_dir_for_loadfiles(const char *path_directory);


void movie_start_command();


void movie_pause_command();


void movie_end_command();


// x0, y0 are offset value, but you set the value to negative, the image is centered.
movie_err_t start_movie(char *path_mjpg, char *path_mp3, int16_t x0, int16_t y0, UBaseType_t taskPriority);


// x0, y0 are offset value, but you set the value to negative, the image is centered.
movie_err_t play_movie(char *path_mjpg, char *path_mp3, int16_t x0, int16_t y0);

#ifdef __cplusplus
}
#endif