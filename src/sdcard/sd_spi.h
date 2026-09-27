// SD Card(SPI) Setting for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <driver/gpio.h>
#include <esp_vfs_fat.h>
#include <ffconf.h>

#include "USER_SETTING.h"

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t init_sd();

size_t file_size(const char *path);

// 1:exist, 0:not exist
bool file_exist(char *path);

// 1:readable, 0:not readable or not exist
bool file_readable(char *path);

// 1:writable, 0:not writable or not exist
bool file_writable(char *path);

#ifdef __cplusplus
}
#endif
