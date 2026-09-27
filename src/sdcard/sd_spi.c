// SD Card(SPI) Setting for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "sd_spi.h"


esp_err_t init_sd()
{
    esp_err_t err;

    // SPI Bus Setting
    spi_bus_config_t spi_cfg = {
        .mosi_io_num = PIN_SD_SPI_MOSI,
        .miso_io_num = PIN_SD_SPI_MISO,
        .sclk_io_num = PIN_SD_SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4092,
    };
    
    // SD Card Setting
    sdspi_device_config_t sd_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    sd_cfg.host_id = SD_SPI_HOST;
    sd_cfg.gpio_cs = PIN_SD_SPI_CS;

    // FAT File System Setting
    esp_vfs_fat_sdmmc_mount_config_t fs_cfg = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SD_SPI_FREQ_kHz;
    host.slot = SD_SPI_HOST;
    host.command_timeout_ms = 200;

    sdmmc_card_t *card;

    err = spi_bus_initialize(SD_SPI_HOST, &spi_cfg, SPI_DMA_CH_AUTO);
    if(err) return err;

    err = esp_vfs_fat_sdspi_mount(PATH_SD, &host, &sd_cfg, &fs_cfg, &card);
    if(err) return err;

    return ESP_OK;
}


size_t file_size(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0) {
        //printf("Stat file error.\n");
        return 0;
    }

    return st.st_size;
}


// 1:exist, 0:not exist
bool file_exist(char *path)
{
    if(access(path, F_OK) == 0) return 1; // exist
    return 0; // not exist
}


// 1:readable, 0:not readable or not exist
bool file_readable(char *path)
{
    if(access(path, R_OK) == 0) return 1; // readable
    return 0; // not readable or not exist
}


// 1:writable, 0:not writable or not exist
bool file_writable(char *path)
{
    if(access(path, W_OK) == 0) return 1; // writable
    return 0; // not writable or not exist
}

