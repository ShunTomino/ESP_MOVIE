// SD Card(SPI) Setting for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "sdcard.h"

#if (SD_MODE == 0)
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
    sd_cfg.host_id = (spi_host_device_t)SD_SPI_HOST; // SPI2_HOST:1, SPI3_HOST:2
    sd_cfg.gpio_cs = (gpio_num_t)PIN_SD_SPI_CS;

    // FAT File System Setting
    esp_vfs_fat_sdmmc_mount_config_t fs_cfg = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SD_SPI_FREQ_kHz;
    host.slot = (spi_host_device_t)SD_SPI_HOST;
    host.command_timeout_ms = 200;

    sdmmc_card_t *card;

    err = spi_bus_initialize((spi_host_device_t)SD_SPI_HOST, &spi_cfg, SPI_DMA_CH_AUTO);
    if(err) return err;

    err = esp_vfs_fat_sdspi_mount(PATH_SD, &host, &sd_cfg, &fs_cfg, &card);
    if(err) return err;

    return ESP_OK;
}
#else
esp_err_t init_sd()
{
    esp_err_t err;

    // SD Card Setting
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_kHz;
    host.slot = SDMMC_HOST_SLOT_1;

    sdmmc_slot_config_t sd_cfg = SDMMC_SLOT_CONFIG_DEFAULT();
    //sd_cfg.width = SDMMC_BUS_WIDTH;
    sd_cfg.width = 1; // Currently, only 1-line mode is supported.

    sd_cfg.clk = (gpio_num_t)PIN_SDMMC_CLK;
    sd_cfg.cmd = (gpio_num_t)PIN_SDMMC_CMD;
    sd_cfg.d0  = (gpio_num_t)PIN_SDMMC_D0;
    //sd_cfg.d1  = (gpio_num_t)PIN_SDMMC_D1;
    //sd_cfg.d2  = (gpio_num_t)PIN_SDMMC_D2;
    //sd_cfg.d3  = (gpio_num_t)PIN_SDMMC_D3;
    //sd_cfg.d4  = (gpio_num_t)PIN_SDMMC_D4;
    //sd_cfg.d5  = (gpio_num_t)PIN_SDMMC_D5;
    //sd_cfg.d6  = (gpio_num_t)PIN_SDMMC_D6;
    //sd_cfg.d7  = (gpio_num_t)PIN_SDMMC_D7;
    sd_cfg.cd  = (gpio_num_t)PIN_SDMMC_CD;
    sd_cfg.wp  = (gpio_num_t)PIN_SDMMC_WP;

    // FAT File System Setting
    esp_vfs_fat_sdmmc_mount_config_t fs_cfg = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
    };

    sdmmc_card_t *card;

    err = esp_vfs_fat_sdmmc_mount(PATH_SD, &host, &sd_cfg, &fs_cfg, &card);
    if(err) return err;

    return ESP_OK;
}
#endif


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

