// LCD Driver for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "Bus_i80.h"


// Buffer for pixcel data
#if JPG_BLOCK_DEC_ENABLE
    DMA_ATTR __attribute__((aligned(16))) uint16_t imgBuf[IMGBUF_COUNT][IMGBUF_LEN];

    uint32_t IMGBUF_LEN_MAX = IMGBUF_LEN;
    size_t IMGBUF_SIZE_MAX  = IMGBUF_LEN * 2;
    volatile bool imgBuf_busy[IMGBUF_COUNT] = {false, false};

    typedef struct {
        uint8_t queue[IMGBUF_COUNT]; // queue for imgBuf number
        uint8_t in_pos; // push position
        uint8_t out_pos; // pop position
    } buf_num_queue_t;

    volatile buf_num_queue_t buf_num = {
        .in_pos = 0,
        .out_pos = 0,
    };

#else
    DMA_ATTR __attribute__((aligned(16))) uint16_t imgBuf[IMGBUF_LEN];

    uint32_t IMGBUF_LEN_MAX = IMGBUF_LEN;
    size_t IMGBUF_SIZE_MAX  = IMGBUF_LEN * 2;
    volatile bool imgBuf_busy = false;
    
    #if (IMGBUF_LEN * 2 > MAX_DMA_TRANSFER_SIZE)
        size_t max_transfer_size = MAX_DMA_TRANSFER_SIZE;
    #endif
#endif

uint32_t IMG_LEN = LCD_WIDTH * LCD_HEIGHT;
size_t IMG_SIZE = LCD_WIDTH * LCD_HEIGHT * 2; // RGB565:2byte/pixel

esp_lcd_i80_bus_handle_t i80_bus = NULL;
esp_lcd_panel_io_handle_t i80_handle = NULL;


// 8bit Command Sending
void send_command(uint8_t cmd)
{
    //esp_err_t ret = 
    esp_lcd_panel_io_tx_param(i80_handle, cmd, NULL, 0);
    //printf("cmd send : %s", esp_err_to_name(ret));
}


// 8bit Data Sending
void send_data(uint8_t data)
{
    //esp_err_t ret = 
    esp_lcd_panel_io_tx_param(i80_handle, -1, &data, 1);
    //printf("data send : %s", esp_err_to_name(ret));
}


// Callback function invoked when Sending the image data by DMA is finished
bool lcd_dma_send_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    #if JPG_BLOCK_DEC_ENABLE
        //ESP_EARLY_LOGI("Send_done","buf:%d\n",buf_num.queue[buf_num.out_pos]);
        imgBuf_busy[buf_num.queue[buf_num.out_pos]] = false;
        buf_num.out_pos ^= 1; // Chanege output of queue position
    #else
        imgBuf_busy = false; // Sending done
    #endif

    return false;
}


#if JPG_BLOCK_DEC_ENABLE
void set_double_image_buffer(uint16_t **img_buf, volatile bool **double_img_buf_busy)
{
    static uint8_t img_buf_current = 0;
    
    // Set imgBuf number to queue
    buf_num.queue[buf_num.in_pos] = img_buf_current; // Push
    buf_num.in_pos ^= 1; // Chanege input of queue position

    // Set imgBuf
    *img_buf = imgBuf[img_buf_current]; // Set output buffer

    // Get pointer for sending state
    *double_img_buf_busy = &imgBuf_busy[img_buf_current];

    // Change output buffer
    img_buf_current ^= 1;
}
#endif


// Intel8080 8bit pararel bus Initialization
esp_err_t init_i80()
{
    esp_err_t err;

    esp_lcd_i80_bus_config_t i80_bus_cfg = {
        .dc_gpio_num = PIN_LCD_DC,
        .wr_gpio_num = PIN_LCD_WR,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_gpio_nums = {
            PIN_LCD_D0,
            PIN_LCD_D1,
            PIN_LCD_D2,
            PIN_LCD_D3,
            PIN_LCD_D4,
            PIN_LCD_D5,
            PIN_LCD_D6,
            PIN_LCD_D7,
        },
        .bus_width = 8, // pararel 8bit

        #if (IMGBUF_LEN * 2 > MAX_DMA_TRANSFER_SIZE)
            .max_transfer_bytes = MAX_DMA_TRANSFER_SIZE,
        #else
            .max_transfer_bytes = IMGBUF_SIZE_MAX,
        #endif

        .dma_burst_size = 32,
    };

    esp_lcd_panel_io_i80_config_t i80_io_cfg = {
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = LCD_CLK_FREQ,
        .trans_queue_depth = IMGBUF_COUNT,
        .on_color_trans_done = lcd_dma_send_done,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        
        .dc_levels = {
            .dc_idle_level  = 1, // Idle Phase:DC=1
            .dc_cmd_level   = 0, // Command Phase:DC=0
            .dc_dummy_level = 1, // Dummy Phase:DC=1
            .dc_data_level  = 1, // Data Phase:DC=1
        },
        
        .flags = {
            .cs_active_high     = 0,
            .reverse_color_bits = 0,
            .swap_color_bytes   = 1,
            .pclk_active_neg    = 0,
            .pclk_idle_low      = 0,
        },
        
    };
    

    // Reset Pin Setting
    gpio_reset_pin(PIN_LCD_D0);
    gpio_reset_pin(PIN_LCD_D1);
    gpio_reset_pin(PIN_LCD_D2);
    gpio_reset_pin(PIN_LCD_D3);
    gpio_reset_pin(PIN_LCD_D4);
    gpio_reset_pin(PIN_LCD_D5);
    gpio_reset_pin(PIN_LCD_D6);
    gpio_reset_pin(PIN_LCD_D7);
    gpio_reset_pin(PIN_LCD_WR);
    gpio_reset_pin(PIN_LCD_DC);

    err = esp_lcd_new_i80_bus(&i80_bus_cfg, &i80_bus);
    if (err) return err;
    
    err = esp_lcd_new_panel_io_i80(i80_bus, &i80_io_cfg, &i80_handle);
    if (err) return err;

    // RD Pin Setting (But currently, READ MODE is not supported.)
    gpio_reset_pin(PIN_LCD_RD);
    gpio_set_direction(PIN_LCD_RD, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_LCD_RD, 1);

    send_data(0x00); // Dummy data for first sending

    return ESP_OK;
}


esp_err_t deinit_i80(){
    return esp_lcd_del_i80_bus(i80_bus);
}