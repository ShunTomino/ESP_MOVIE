// LCD Driver for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "lcd_driver.h"


/*
// Positive Gamma Correction (0xE0)
uint8_t gamma_pos[15] = {0x00,0x0C,0x11,0x04,0x11,0x08,0x37,0x89,0x4C,0x06,0x0C,0x0A,0x2E,0x34,0x0F};
// Negative Gamma Correction (0xE1)
uint8_t gamma_neg[15] = {0x00,0x0B,0x11,0x05,0x13,0x09,0x33,0x67,0x48,0x07,0x0E,0x0B,0x2E,0x33,0x0F};
*/


// LCD Back Light PWM Initialization
esp_err_t init_backlight()
{
    esp_err_t err;

    ledc_timer_config_t ledc_cfg = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_8_BIT, // 0〜255
        .timer_num        = LEDC_PWM_TIMER_NUM,
        .freq_hz          = 20000,
        .clk_cfg          = LEDC_AUTO_CLK,
    };

    ledc_channel_config_t ledc_ch_chg = {
        .gpio_num   = PIN_LCD_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_PWM_CHANNEL,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_PWM_TIMER_NUM,
        .duty       = 0,
        .hpoint     = 0,
        .flags = {
          .output_invert = 0
        }
    };
    
    err = ledc_timer_config(&ledc_cfg);
    if (err) return err;

    err = ledc_channel_config(&ledc_ch_chg);
    if (err) return err;

    // Set LCD Back LIght Volume
    backlight_volume(0);

    return ESP_OK;
}


// LCD Back Light Volume Control : 0〜255
esp_err_t backlight_volume(uint8_t volume)
{
    esp_err_t err;
    err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_PWM_CHANNEL, volume);
    if(err) return err;

    return ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_PWM_CHANNEL);
}


// Set the window area
// you don't need to send pixel data outside the specified range.
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t x[4];
    uint8_t y[4];

    x[0] = x0 >> 8; x[1] = x0 & 0xFF; // Start column
    x[2] = x1 >> 8; x[3] = x1 & 0xFF; // End column
    y[0] = y0 >> 8; y[1] = y0 & 0xFF; // Start row
    y[2] = y1 >> 8; y[3] = y1 & 0xFF; // End row

    esp_lcd_panel_io_tx_param(i80_handle, 0x2A, x, 4); // x Address Set
    esp_lcd_panel_io_tx_param(i80_handle, 0x2B, y, 4); // y Address Set

    /*
    send_command(0x2A); // x Address Set
    send_data(x0 >> 8); send_data(x0 & 0xFF); // Start column
    send_data(x1 >> 8); send_data(x1 & 0xFF); // End column

    send_command(0x2B); // y Address Set
    send_data(y0 >> 8); send_data(y0 & 0xFF); // Start row
    send_data(y1 >> 8); send_data(y1 & 0xFF); // End row
    */
}


// Set the window area for displaying an image
// x0, y0 are offset value, but you set the value to negative, the image is centered.
// <return> 0:OK, -1:Over Width Error, -2:Over Height Error
int8_t set_image_window(uint16_t image_width, uint16_t image_height, int16_t x0, int16_t y0)
{
    uint16_t offset_x, offset_y;
    int16_t  blank_x, blank_y;

    // Set display area
    blank_x = LCD_WIDTH - image_width;
    blank_y = LCD_HEIGHT - image_height;
    if(blank_x < 0) return -1; // Over Width Error
    if(blank_y < 0) return -2; // Over Heigth Error

    if(x0 < 0) offset_x = (blank_x + 1)/2;
    else offset_x = (x0 > blank_x) ? blank_x : x0;
    if(y0 < 0) offset_y = (blank_y + 1)/2;
    else offset_y = (y0 > blank_y) ? blank_y : y0;

    set_window(offset_x, offset_y, offset_x+image_width-1, offset_y+image_height-1);
    return 0; // OK
}


// Set the X/Y axis of the display
// <x_inverse> 0:Normal, 1:Inverse 
// <y_inverse> 0:Normal, 1:Inverse
// <xy_exchange> 0:Normal, 1:Exchange
void set_axis(bool x_inverse, bool y_inverse, bool xy_exchange)
{
    uint8_t madctl = 0; // MADCTL register value

    madctl = (y_inverse << 7) | (x_inverse << 6) | (xy_exchange << 5) | ((bool)LCD_RGB_FORMAT << 3);

    send_command(0x36); send_data(madctl);
}


#if LCD_COLOR_TABLE_ENABLE
// Set Color Volume Look Up Table (RGB565->RGB666 Color depth convertion in LCD)
// <gamma_curve> 0:Gamma 4.0, 1:Gamma 3.0, 2:Gamma 2.0, 3:Gamma 1.0
// <color_vol_max> Maximum color volume (0~63)
// <color_vol_output_offset> Color volume offset (0~63)
void set_color_table(uint8_t gamma_curve, uint8_t color_vol_max, uint8_t color_vol_offset)
{
    uint8_t red_table[32];   // R: 32byte
    uint8_t green_table[64]; // G: 64byte
    uint8_t blue_table[32];  // B: 32byte
    uint64_t i, max;

    max = (color_vol_max - color_vol_offset);

    // 6-bit Color Table (RGB565->RGB666), 0~63
    switch(gamma_curve){
        case 0: // Gamma 4.0
            for(i=1; i<32; i++)red_table[i]   = (i * i * i * i * max + 461760) / 923521 + color_vol_offset;
            for(i=1; i<64; i++)green_table[i] = (i * i * i * i * max + 7876531) / 15752961 + color_vol_offset;
            for(i=1; i<32; i++)blue_table[i]  = (i * i * i * i * max + 461760) / 923521 + color_vol_offset;
            break;
        case 1: // Gamma 3.0
            for(i=1; i<32; i++)red_table[i]   = (i * i * i * max + 14895) / 29791 + color_vol_offset;
            for(i=1; i<64; i++)green_table[i] = (i * i * i * max + 125000) / 250047 + color_vol_offset;
            for(i=1; i<32; i++)blue_table[i]  = (i * i * i * max + 14895) / 29791 + color_vol_offset;
            break;
        case 2: // Gamma 2.0
            for(i=1; i<32; i++)red_table[i]   = (i * i * max + 480) / 961 + color_vol_offset;
            for(i=1; i<64; i++)green_table[i] = (i * i * max + 1984) / 3969 + color_vol_offset;
            for(i=1; i<32; i++)blue_table[i]  = (i * i * max + 480) / 961 + color_vol_offset;
            break;
        default: // Gamma 1.0
            for(i=1; i<32; i++)red_table[i]   = (i * max + 15) / 31 + color_vol_offset;
            for(i=1; i<64; i++)green_table[i] = (i + color_vol_offset > max) ? max : (i + color_vol_offset);
            for(i=1; i<32; i++)blue_table[i]  = (i * max + 15) / 31 + color_vol_offset;
    }

    // Black Table
    red_table[0] = 0;
    green_table[0] = 0;
    blue_table[0] = 0;

    esp_lcd_panel_io_tx_param(i80_handle, 0x2D, red_table, 32); // Set RED table
    esp_lcd_panel_io_tx_param(i80_handle,   -1, green_table, 32); // Set GREEN table 1
    esp_lcd_panel_io_tx_param(i80_handle,   -1, green_table+32, 32); // Set GREEN table 2
    esp_lcd_panel_io_tx_param(i80_handle,   -1, blue_table, 32); // Set BLUE table

    vTaskDelay(10 / portTICK_PERIOD_MS); // Wait for LCD ready
}
#endif


esp_err_t init_display()
{
    esp_err_t err;
    //uint16_t i,j;

    // Reset Display
    gpio_reset_pin(PIN_LCD_RST);
    gpio_set_direction(PIN_LCD_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_LCD_RST, 0); // LED OFF
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_LCD_RST, 1); // LED ON
    vTaskDelay(120 / portTICK_PERIOD_MS);

    // 8bit Parallel Bus Initialization
    err = init_i80();
    if (err) return err;

    // Send initialization commands
    send_command(0x28); // Display OFF
    
    send_command(0xCF); send_data(0x00); send_data(0x83); send_data(0x30);
    send_command(0xED); send_data(0x64); send_data(0x03); send_data(0x12); send_data(0x81);
    send_command(0xE8); send_data(0x85); send_data(0x01); send_data(0x79);
    send_command(0xCB); send_data(0x39); send_data(0x2C); send_data(0x00); send_data(0x34); send_data(0x02);
    send_command(0xF7); send_data(0x20);
    send_command(0xEA); send_data(0x00); send_data(0x00);
    
    /*
    // Power control settings
    send_command(0xC0); send_data(0x03); send_data(0x00); // Power Control 1
    send_command(0xC1); send_data(0x70); // Power Control 2
    send_command(0xC2); send_data(0x44); // Power Control 3
    send_command(0xC5); send_data(0x00); send_data(0x14);
    send_command(0xC7); send_data(0xFF);
    */

    send_command(0x3A); send_data(0x55); // Pixel data size (16-bit)

    // Frame rate control
    send_command(0xB1); send_data(0x00); send_data(0x1B);

    // Display function control
    send_command(0xB6); send_data(0x0A); send_data(0x82); send_data(0x27); send_data(0x00);

    // Color Inverse
    #if LCD_DATA_INVERSE
        send_command(0x21); // Inverse ON:0x21
    #else
        send_command(0x20); // Inverse OFF:0x20
    #endif

    // Gumma set
    send_command(0x26); send_data(0x04);

    /*
    // Gumma Curve
    send_command(0xE0); // positive gamma
    for(i=0; i<15; i++)send_data(gamma_pos[i]);
    send_command(0xE1); // negative gamma
    for(i=0; i<15; i++)send_data(gamma_neg[i]);
    */

    #if LCD_COLOR_TABLE_ENABLE
        set_color_table(GAMMA_CURVE, COLOR_VOL_MAX, COLOR_VOL_OFFSET);
    #endif

    // ----------Enable display----------
    send_command(0x11); // Sleep OUT
    vTaskDelay(120 / portTICK_PERIOD_MS);

    send_command(0x38); // Idle Mode OFF
    send_command(0x13); // Normal Display Mode ON
    send_command(0x35); // Tearing Effect Line OFF

    send_command(0x29); // Display ON

    set_axis(LCD_X_INVERSE, LCD_Y_INVERSE, LCD_XY_EXCHANGE);
    set_window(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);

    return ESP_OK;
}


#if !JPG_BLOCK_DEC_ENABLE
// Sending all of the image buffer data by DMA
void send_image(size_t size)
{
    #if (IMGBUF_LEN * 2 > MAX_DMA_TRANSFER_SIZE)
        uint8_t *img_buf = (uint8_t *)imgBuf;
        size_t block_size;
        uint16_t block_count;
        uint16_t remain_size;
        uint16_t i;

        if(size == 0) return;
        block_size = (size < max_transfer_size) ? size : max_transfer_size;
        block_count = size / block_size;
        remain_size = size % block_size;
        
        esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, img_buf, block_size);
        img_buf += block_size;
        for(i = 1; i < block_count; i++){
            esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, block_size);
            img_buf += block_size;
        }
        esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, remain_size);
        
    #else
        esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, imgBuf, size);
    #endif
}

// Set a specific color in a rectangular area of the image buffer
// Not sent to the LCD yet. Use send_image() to send it to the LCD.
void set_color(uint16_t rgb565, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t *buf = LCD_WIDTH * y0 + imgBuf;
    uint16_t x, y;

    x1 = x1 <  LCD_WIDTH ? x1 : (LCD_WIDTH-1);
    y1 = y1 < LCD_HEIGHT ? y1 : (LCD_HEIGHT-1);

    for(y=y0; y<=y1; y++) {
        for(x=x0; x<=x1; x++) buf[x] = rgb565;
        buf += LCD_WIDTH;
    }
}
#endif


// Fill the entire image buffer with a specific color.
// Not sent to the LCD yet. 
void set_color_all(uint16_t rgb565)
{
  uint32_t i;

    #if JPG_BLOCK_DEC_ENABLE
        if((rgb565==RGB565_BLACK)|(rgb565==RGB565_WHITE)) {
            memset(imgBuf[0], rgb565>>8, IMGBUF_SIZE_MAX);
            memset(imgBuf[1], rgb565>>8, IMGBUF_SIZE_MAX);
        } else {
            for (i=0; i < IMGBUF_LEN_MAX; i++) {
                imgBuf[0][i] = rgb565;
                imgBuf[1][i] = rgb565;
            }
        }
    #else
        if((rgb565==RGB565_BLACK)|(rgb565==RGB565_WHITE)) memset(imgBuf, rgb565>>8, IMGBUF_SIZE_MAX);
        else for (i=0; i < IMGBUF_LEN_MAX; i++) imgBuf[i] = rgb565;
    #endif
}


// Display a specific color on the entire LCD screen
void display_color_all(uint16_t rgb565)
{
    #if JPG_BLOCK_DEC_ENABLE
        volatile bool *double_img_buf_busy;
        uint16_t *img_buf;
        uint32_t count = IMG_SIZE / IMGBUF_SIZE_MAX;
        uint32_t remain = IMG_SIZE % IMGBUF_SIZE_MAX;
        uint32_t i;
    #endif

    set_window(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
    set_color_all(rgb565);

    #if JPG_BLOCK_DEC_ENABLE
        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, img_buf, IMGBUF_SIZE_MAX);
        for (i=1; i < count; i++){
            set_double_image_buffer(&img_buf, &double_img_buf_busy);
            esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, IMGBUF_SIZE_MAX);
        }
        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, remain);
    #else
        send_image(IMGBUF_SIZE_MAX);
    #endif
}


// Display a specific color in a rectangular area of the LCD screen
void display_color(uint16_t rgb565, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    size_t img_size = (x1 - x0 + 1) * (y1 - y0 + 1) * 2;

    #if JPG_BLOCK_DEC_ENABLE
        volatile bool *double_img_buf_busy;
        uint16_t *img_buf;
        uint32_t count = img_size / IMGBUF_SIZE_MAX;
        uint32_t remain = img_size % IMGBUF_SIZE_MAX;
        uint32_t i;
    #endif

    x1 = x1 <  LCD_WIDTH ? x1 : (LCD_WIDTH-1);
    y1 = y1 < LCD_HEIGHT ? y1 : (LCD_HEIGHT-1);

    set_window(x0, y0, x1, y1);
    set_color_all(rgb565);

    #if JPG_BLOCK_DEC_ENABLE
        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, img_buf, IMGBUF_SIZE_MAX);
        for (i=1; i < count; i++){
            set_double_image_buffer(&img_buf, &double_img_buf_busy);
            esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, IMGBUF_SIZE_MAX);
        }
        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, remain);
    #else
        send_image(img_size);
    #endif
}


// For test sending without DMA
void display_color_all_NoDMA(uint16_t rgb565)
{
    uint32_t i;

    send_command(PIXEL_TX_CMD); // LCD Memory Write Command

    // Pixel Data Sending
    for (i=0; i<IMG_LEN; i++){
        send_data(rgb565 >> 8);
        send_data(rgb565 & 0xFF);
    }
}



#if !JPG_BLOCK_DEC_ENABLE
// Set a string in the image buffer with a specific color at a given position
// Not sent to the LCD yet. Use send_image() to send it to the LCD.
// <return> 0:OK, minus value:Sting length over
int8_t set_string(const char *str, uint16_t rgb565, uint16_t lcd_x, uint16_t lcd_y)
{
    uint16_t *buf0 = lcd_y * LCD_WIDTH + lcd_x + imgBuf;
    uint16_t *buf;
    uint8_t *font;
    size_t len = strlen(str);
    uint16_t len_max;
    int16_t i;
    uint8_t x, y;

    if(lcd_x > LCD_WIDTH-1)return -1;
    if(lcd_y > LCD_HEIGHT-17)return -2;

    len_max = (LCD_WIDTH - lcd_x - 1) / 8;

    for(i=0; (i<len)&&(i<len_max); i++) {

        font = str[i] * 16 + (uint8_t *)font8x16;
        buf = 8*i + buf0;

        for(y=0; y<16; y++) {
            for (x=0; x<8; x++) {
                if(*font & (0x80 >> x)) buf[x] = rgb565;
            }
            buf += LCD_WIDTH;
            font++;
        }
    }
    return 0;
}
#endif


// Display a string on the LCD with a specific font color and background color at a given position
// <return> 0:OK, minus value:Sting length over
int8_t display_string(const char *str, uint16_t rgb565_font, uint16_t rgb565_back, uint16_t x0, uint16_t y0)
{
    uint16_t *buf;
    uint8_t *font;
    uint16_t width;
    uint16_t len_max;
    uint16_t len = strlen(str);
    uint8_t i, x, y;

    #if JPG_BLOCK_DEC_ENABLE
        volatile bool *double_img_buf_busy;
        uint16_t *img_buf;
    #else
        uint16_t *img_buf = imgBuf;
    #endif

    if(x0 + 8 > LCD_WIDTH)return -1;   // Over Width
    if(y0 + 16 > LCD_HEIGHT)return -2; // Over Heigth

    len_max = (LCD_WIDTH - x0) / 8;

    len = len > len_max ? len_max : len;

    width = len * 8;

    set_window(x0, y0, x0 + width - 1, y0 + 15);

    #if JPG_BLOCK_DEC_ENABLE
        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        while(*double_img_buf_busy); // Wait for buffer to finish sending
        *double_img_buf_busy = true;
    #else
        while(imgBuf_busy);
        imgBuf_busy = true;
    #endif

    for(i=0; i < len; i++) {
        font = str[i] * 16 + (uint8_t *)font8x16;
        buf = 8*i + img_buf;

        for(y=0; y<16; y++) {
            for (x=0; x<8; x++) buf[x] = (*font & (0x80 >> x)) ? rgb565_font : rgb565_back;
            buf += width;
            font++;
        }
    }

    #if JPG_BLOCK_DEC_ENABLE
        esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, img_buf, width * 32);
    #else
        send_image(width * 32);
    #endif

    return 0; // OK
}
