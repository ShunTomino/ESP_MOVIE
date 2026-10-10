// JPG/MJPG Decoder for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "JPG_decoder.h"


#if JPG_BLOCK_DEC_ENABLE
    #if JPG_BUF_ENABLE_PSRAM
        EXT_RAM_BSS_ATTR uint8_t jpg_buf[JPG_BUF_SIZE]; // PSRAM
    #else
        uint8_t jpg_buf[JPG_BUF_SIZE];
    #endif
#else
    EXT_RAM_BSS_ATTR uint8_t jpg_buf[JPG_BUF_SIZE]; // PSRAM
#endif

char path_mjpgload[MAX_PATH_LEN];

jpeg_dec_handle_t jpg_dec;
jpeg_dec_config_t jpg_dec_cfg;
jpeg_dec_io_t jpg_io;

SemaphoreHandle_t mjpg_dec_standby; // Semaphore for decode standby
SemaphoreHandle_t mjpg_dec_start; // Semaphore for decode start
SemaphoreHandle_t mjpg_dec_end; // Semaphore for decode end

bool mjpgContinue = true; // Continue command (true:continue, false:end)
bool mjpgPause = false; // Pause command (true:pause, false:resume)


// MJPGバックグラウンド再生終了時のコールバック関数
static FuncEndMJPG func_end_mjpg = NULL;


jpg_err_t init_jpg_decoder(FuncEndMJPG func_end)
{
    // Decoder configuration
    jpg_dec_cfg.output_type = JPEG_PIXEL_FORMAT_RGB565_LE;
    jpg_dec_cfg.rotate = JPEG_ROTATE_0D;
    jpg_dec_cfg.scale.width    = 0; // No need to change
    jpg_dec_cfg.scale.height   = 0; // No need to change
    jpg_dec_cfg.clipper.width  = 0; // No need to change
    jpg_dec_cfg.clipper.height = 0; // No need to change
    //jpg_dec_cfg.clipper.width  = LCD_WIDTH;
    //jpg_dec_cfg.clipper.height = LCD_HEIGHT;
    
    #if JPG_BLOCK_DEC_ENABLE
        jpg_dec_cfg.block_enable = 1;
    #else
        jpg_dec_cfg.block_enable = 0;
    #endif
    
    // CallBack Registration
    func_end_mjpg = func_end;
    
    set_dir_for_mjpgload(path_mjpgload, PATH_SD);

    mjpg_dec_start = xSemaphoreCreateBinary();

    // Create jpg_dec handle
    return (jpg_err_t)jpeg_dec_open(&jpg_dec_cfg, &jpg_dec);
}


jpg_err_t deinit_jpg_decoder()
{
    return (jpg_err_t)jpeg_dec_close(jpg_dec);
}


// x0,y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t display_jpg(char *path_jpg, int16_t x0, int16_t y0, jpeg_dec_header_info_t *jpg_info)
{
    FILE *file_jpg;
    jpeg_error_t err;

    #if JPG_BLOCK_DEC_ENABLE
        volatile bool *double_img_buf_busy;
        uint16_t *img_buf;
        int block_size = 0;
        int block_count = 0;
        int remain_size = 0;
        uint16_t i;
    #endif


    file_jpg = fopen(path_jpg, "rb");
    if (!file_jpg) {
        printf("Failed to open file: %s\n", path_jpg);
        printf("SDK Config FF_MAX_LFN: %d\n", FF_MAX_LFN);
        return -8;
    }

    fread(jpg_buf, 1, JPG_BUF_SIZE, file_jpg);
    fclose(file_jpg);

    // Set input/output buffer and input buffer length
    jpg_io.inbuf = jpg_buf;
    jpg_io.inbuf_len = JPG_BUF_SIZE;
    
    #if JPG_BLOCK_DEC_ENABLE
        jpg_io.out_size = IMGBUF_LEN; // Set output buffer length(Only for block decode mode)
    #endif

    // Parse jpeg picture header and get picture for user and decoder
    err = jpeg_dec_parse_header(jpg_dec, &jpg_io, jpg_info);
    if (err) return (jpg_err_t)err;

    // Set image size
    err = set_image_window(jpg_info->width, jpg_info->height, x0, y0);
    if (err) return JPG_ERR_IMAGE_SIZE_OVER;


    #if JPG_BLOCK_DEC_ENABLE
        err = jpeg_dec_get_outbuf_len(jpg_dec, &block_size);
        if (err) return (jpg_err_t)err;

        err = jpeg_dec_get_process_count(jpg_dec, &block_count); // Get process count
        if (err) return (jpg_err_t)err;

        remain_size = (jpg_info->width * jpg_info->height * 2) % block_size;
        if (remain_size == 0) remain_size = block_size;

        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        jpg_io.outbuf = (uint8_t *)img_buf; // Set output buffer
        while(*double_img_buf_busy); // Wait for buffer to finish sending
        *double_img_buf_busy = true;
        err = jpeg_dec_process(jpg_dec, &jpg_io);
        if(err) return (jpg_err_t)err;
        esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, img_buf, block_size); // Initial sending
        
        for (i = 2; i < block_count; i++) {
            set_double_image_buffer(&img_buf, &double_img_buf_busy);
            jpg_io.outbuf = (uint8_t *)img_buf; // Set output buffer
            while(*double_img_buf_busy); // Wait for buffer to finish sending
            *double_img_buf_busy = true;
            jpeg_dec_process(jpg_dec, &jpg_io);
            esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, block_size);
        }
        set_double_image_buffer(&img_buf, &double_img_buf_busy);
        jpg_io.outbuf = (uint8_t *)img_buf; // Set output buffer
        while(*double_img_buf_busy); // Wait for buffer to finish sending
        *double_img_buf_busy = true;
        jpeg_dec_process(jpg_dec, &jpg_io);
        esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, remain_size);
    #else
        jpg_io.outbuf = (uint8_t *)imgBuf; // Set output buffer
        while(imgBuf_busy);
        imgBuf_busy = true;
        err = jpeg_dec_process(jpg_dec, &jpg_io);
        if (err) {
            if(file_size(path_jpg) > JPG_BUF_SIZE) printf("JPGBUF_SIZE is not enough.\n");
            return (jpg_err_t)err;
        }
        send_image(jpg_info->width * jpg_info->height * 2);
    #endif
    
    return JPG_OK;
}


// Set path of directory for mjpgload file
void set_dir_for_mjpgload(char *path_mjpgload, const char *path_directory)
{    
	memset(path_mjpgload, 0, MAX_PATH_LEN);
    strncpy(path_mjpgload, path_directory, MAX_PATH_LEN);
	strlcat(path_mjpgload, "/", MAX_PATH_LEN);
}


// Set path for mjpgload file to path_mjpgload
int8_t get_path_mjpgload(char *path_mjpgload, const char *path_mjpg)
{
    char path_mjpg_copy[MAX_PATH_LEN];
    char *file_name = NULL;
    char *dot = NULL;
	char *dir_end = NULL;

    strlcpy(path_mjpg_copy, path_mjpg, MAX_PATH_LEN);

    file_name = strrchr(path_mjpg_copy, '/'); // Get last '/' address
    if(file_name) file_name++;
    else return -1;

    dot = strrchr(path_mjpg_copy, '.'); // Get last '.' address
    if(dot) *dot = '\0';
    else return -1;

    dir_end = strrchr(path_mjpgload, '/'); // Get last '/' address
	if(dir_end) *dir_end = '\0';

    strlcat(path_mjpgload, "/", MAX_PATH_LEN);
    strlcat(path_mjpgload, file_name, MAX_PATH_LEN);
    strlcat(path_mjpgload, ".mjpgload", MAX_PATH_LEN);
    
    return 0;
}


// Get AVI file information
// OK: return 0, NG: return minus value below.
// -1: RIFF Container format NG
// -2: AVI format NG
// -3: VIDEO type NG
// -4: MJPG Compression format NG
int8_t get_avi_info(avi_info_t *avi_info, FILE *mjpg_file)
{
    uint8_t RIFF[4];
    uint8_t AVI[4];
    uint8_t fccType[4];
    uint8_t fccHandler[4];
    uint8_t is_RIFF = 0;
    uint8_t is_AVI  = 0;
    uint8_t is_vids = 0;
    uint8_t is_MJPG = 0;

    int8_t ret = 0;

    fseek(mjpg_file, 0, SEEK_SET);
    fread(RIFF, 1, 4, mjpg_file);
    is_RIFF = (RIFF[0]=='R')&&(RIFF[1]=='I')&&(RIFF[2]=='F')&&(RIFF[3]=='F');
    
    fseek(mjpg_file, 8, SEEK_SET);
    fread(AVI,  1, 4, mjpg_file);
    is_AVI  = (AVI[0]=='A')&&(AVI[1]=='V')&&(AVI[2]=='I')&&(AVI[3]==' ');

    fseek(mjpg_file, 108, SEEK_SET);
    fread(fccType, 1, 4, mjpg_file);
    is_vids = (fccType[0]=='v')&&(fccType[1]=='i')&&(fccType[2]=='d')&&(fccType[3]=='s');

    fseek(mjpg_file, 112, SEEK_SET);
    fread(fccHandler, 4, 1, mjpg_file);
    is_MJPG = (fccHandler[0]=='M')&&(fccHandler[1]=='J')&&(fccHandler[2]=='P')&&(fccHandler[3]=='G');

    fseek(mjpg_file, 32, SEEK_SET);
    fread(&(avi_info->dwMicroSecPerFrame), 4, 1, mjpg_file);

    fseek(mjpg_file, 48, SEEK_SET);
    fread(&(avi_info->dwTotalFrames), 4, 1, mjpg_file);
    
    fseek(mjpg_file, 64, SEEK_SET);
    fread(&(avi_info->dwWidth),  4, 1, mjpg_file);
    fread(&(avi_info->dwHeight), 4, 1, mjpg_file);

    if(is_RIFF){
        //printf("RIFF Container format OK.\n");
        if(!is_AVI) {
            printf("AVI format NG: %s\n", AVI);
            ret = -1;
        }
    }else{
        printf("RIFF Container format NG: %s\n", RIFF);
        ret = -2;
    }
    
    if(is_vids){
        //printf("VIDEO type OK.\n");
        if(!is_MJPG) {
            printf("MJPG Compression format NG: %s\n", fccHandler);
            ret = -3;
        }
    }else{
        printf("VIDEO type NG: %s\n", fccType);
        ret = -4;
    }

    printf("Micro Sec Per Frame : %ld\n", avi_info->dwMicroSecPerFrame);
    printf("Total Frame: %ld frames\n", avi_info->dwTotalFrames);
    printf("Width:  %ld pixcel\n", avi_info->dwWidth);
    printf("Height: %ld pixcel\n", avi_info->dwHeight);

    fseek(mjpg_file, 0, SEEK_SET);
    return ret;
}


uint32_t search_mjpg_frame(const char *path_mjpg)
{
    FILE *file_mjpg, *file_mjpgload;
    uint32_t search_size = JPG_BUF_SIZE-1;
    uint32_t read_size;
    uint32_t pos = JPG_BUF_SIZE;
    uint32_t remain_size = 0;
    uint32_t offset = 0;
    uint32_t frame_count = 0; // jpg frame count
    uint32_t eoi = 0; // End position Of Image
    uint8_t find_soi = 0;
    uint32_t frame_size = 0;
    uint32_t soi = 0;


    file_mjpg = fopen(path_mjpg, "rb");
	if (!file_mjpg){
		printf("mjpg file open error.\n");
		return 0;
	}

    // Make mjpgload file path
	get_path_mjpgload(path_mjpgload, path_mjpg);

    file_mjpgload = fopen(path_mjpgload, "wb");
    if (file_mjpgload == NULL) {
        printf("Failed to write mjpg load file.\n");
		remove(path_mjpgload);
        return 0;
    }

    
    printf("Start to search the mjpg frame count.\n");

    fseek(file_mjpg, 0, SEEK_SET); // set start point

    while (1) {
        remain_size = JPG_BUF_SIZE - pos;
        memmove(jpg_buf, jpg_buf + pos, remain_size); // 残ったデータを先頭にコピー
        
        //if(frame_count >= MAX_FRAME_SIZE)break;

        read_size = fread(jpg_buf + remain_size, 1, pos, file_mjpg);
        
        if (read_size == 0){ // read_size == 0
            printf("END Of File is found.\n");
            break;
        }
        if (read_size < search_size) search_size = read_size - 1;

        for (pos=0; pos < search_size; pos++) {
            if (jpg_buf[pos] == 0xFF) { 
                if(find_soi && jpg_buf[pos+1] == 0xD9) { // EOI (FFD9)
                    pos++;
                    eoi = offset + pos;
                    //printf("END Of Image position is found: %ld\n", eoi);

                    frame_size = eoi - soi + 1;
                    frame_count++;
                    find_soi = 0;
                    fwrite(&soi, 4, 1, file_mjpgload);
					fwrite(&frame_size,  4, 1, file_mjpgload);

                } else if(jpg_buf[pos+1] == 0xD8) { // SOI (FFD8)
                    soi = offset + pos;
                    pos++;
                    find_soi = 1;
                    //printf("Start Of Image position is found: %ld\n", soi);
                }
            }
        }
        offset += pos;
        //printf("%ld\n", frame_count);
    }

    fclose(file_mjpgload);
	fclose(file_mjpg);
	
	printf("MJPG frame count: %ld\n", frame_count);
    printf("Finish to search the MJPG frame count.\n");
    return frame_count;
}


void mjpg_start_command()
{
    mjpgContinue = true;
	mjpgPause = false;
    xSemaphoreGive(mjpg_dec_start);
}


void mjpg_pause_command()
{
	mjpgContinue = false;
	mjpgPause = true;
}


void mjpg_end_command()
{
	mjpgContinue = false;
	mjpgPause = false;
}


void task_mjpg_decode(void *video_info)
{
    video_info_t video = *(video_info_t *)video_info;
    jpeg_dec_header_info_t jpg_info;
    uint32_t mjpg_frame_info[2];
    FILE *file_mjpg = NULL;
    FILE *file_mjpgload = NULL;
    int32_t start; // microsec
    int32_t mspf_target = video.microsec_per_frame + MJPG_DELAY_PER_FRAME;
    int32_t mspf = mspf_target; // microsec
    int32_t mspf_delay = 0; // microsec

    #if JPG_BLOCK_DEC_ENABLE
        volatile bool *double_img_buf_busy;
        uint16_t *img_buf;
        uint16_t i;
        //uint8_t img_buf_num = 0;
    #endif

    // Open MJPG file
    file_mjpg = fopen(video.path_mjpg, "rb");
    if (!file_mjpg) {
        printf("MJPG open error.\n");
        goto END;
    }

    // Open MJPGLOAD file
    file_mjpgload = fopen(video.path_mjpgload, "rb");
    if (!file_mjpgload) {
        printf("MJPGLOAD open error.\n");
        goto END;
    }

    xSemaphoreGive(mjpg_dec_standby); // Notice MJPG decode standby
    START:
    xSemaphoreTake(mjpg_dec_start, portMAX_DELAY); // Wait for start command
    

    start = esp_timer_get_time();
    video.decoded_time = start;
    
    while((fread(mjpg_frame_info, 4, 2, file_mjpgload) != 0) & mjpgContinue) {
        // Read MJPG data from SD card (Thread safe)
        fseek(file_mjpg, mjpg_frame_info[0], SEEK_SET); // set start point
        fread(jpg_buf, 1, mjpg_frame_info[1], file_mjpg); // read from start point
        
        // MJPG Decode
        jpeg_dec_parse_header(jpg_dec, &jpg_io, &jpg_info);
        
        #if JPG_BLOCK_DEC_ENABLE
            set_double_image_buffer(&img_buf, &double_img_buf_busy);
            jpg_io.outbuf = (uint8_t *)img_buf; // Set output buffer
            while(*double_img_buf_busy); // Wait for buffer to finish sending
            *double_img_buf_busy = true;
            jpeg_dec_process(jpg_dec, &jpg_io);
            esp_lcd_panel_io_tx_color(i80_handle, PIXEL_TX_CMD, img_buf, video.block_size);

            for (i = 2; i < video.block_count; i++) {
                set_double_image_buffer(&img_buf, &double_img_buf_busy);
                jpg_io.outbuf = (uint8_t *)img_buf; // Set output buffer
                while(*double_img_buf_busy); // Wait for buffer to finish sending
                *double_img_buf_busy = true;
                jpeg_dec_process(jpg_dec, &jpg_io);
                esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, video.block_size);
            }
            set_double_image_buffer(&img_buf, &double_img_buf_busy);
            jpg_io.outbuf = (uint8_t *)img_buf; // Set output buffer
            while(*double_img_buf_busy); // Wait for buffer to finish sending
            *double_img_buf_busy = true;
            jpeg_dec_process(jpg_dec, &jpg_io);
            esp_lcd_panel_io_tx_color(i80_handle, -1, img_buf, video.remain_size);
        #else
            while(imgBuf_busy);
            imgBuf_busy = true;
            jpeg_dec_process(jpg_dec, &jpg_io); // Decode MJPG data
            send_image(video.output_size);
        #endif

        // Wait for microsec per frame
        mspf_delay = mspf - mspf_target + mspf_delay;
        mspf = esp_timer_get_time() - start;
        while(mspf_target - mspf_delay > mspf)mspf = esp_timer_get_time() - start;

        start = esp_timer_get_time();
    }

    END:
    video.decoded_time = esp_timer_get_time() - video.decoded_time;
    if(mjpgPause) goto START; // Pause

    // Close files
    if(file_mjpg) fclose(file_mjpg);
    if(file_mjpgload) fclose(file_mjpgload);

    if(func_end_mjpg) func_end_mjpg(&video);
    xSemaphoreGive(mjpg_dec_end); // Notice MJPG decode end
    
    printf("MJPG Task Remaining Stack: %u words\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelete(NULL); // delete this task
}


// x0, y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t set_mjpg(char *path_mjpg, int16_t x0, int16_t y0, const BaseType_t Core0or1, UBaseType_t taskPriority)
{
    avi_info_t avi;
    video_info_t video;
    jpeg_dec_header_info_t jpg_info;
    FILE *file_mjpg = NULL;
    FILE *file_mjpgload = NULL;
    uint32_t mjpg_frame_size  = 0;
    uint32_t mjpg_frame_start = 0;
    jpg_err_t err;

    mjpg_dec_standby = xSemaphoreCreateBinary();
    mjpg_dec_start = xSemaphoreCreateBinary();
    mjpg_dec_end = xSemaphoreCreateBinary();

    // ----------MJPG Decode Initialization----------
    printf("\nMJPG: %s\n", path_mjpg);

    // Set input/output buffer to io_callback
    jpg_io.inbuf = jpg_buf;
    jpg_io.inbuf_len = JPG_BUF_SIZE; // set input buffer length

    #if JPG_BLOCK_DEC_ENABLE
        jpg_io.out_size = IMGBUF_LEN; // Set output buffer length(Only for block decode mode)
    #endif

    // Make mjpgload file path
	get_path_mjpgload(path_mjpgload, path_mjpg);
    //printf("\nMJPGLOAD: %s\n", path_mjpgload);

	// Check mjpgload file existance
	if(file_size(path_mjpgload) < 8) {
	    search_mjpg_frame(path_mjpg); // search frame and make MP3LOAD file
	}
    
    // Open MJPG file
    file_mjpg = fopen(path_mjpg, "rb");
    if (!file_mjpg) {
        printf("Open file error: %s\n", path_mjpg);
        printf("SDK Config FF_MAX_LFN: %d\n", FF_MAX_LFN);
        err = JPG_ERR_OPEN_FILE;
        goto END;
    }

    // Open MJPGLOAD file
    file_mjpgload = fopen(path_mjpgload, "rb");
    if (!file_mjpgload) {
        printf("Open file error: %s\n", path_mjpgload);
        printf("SDK Config FF_MAX_LFN: %d\n", FF_MAX_LFN);
        err = JPG_ERR_OPEN_LOADFILE;
        goto END;
    }
    
    // Set file path
    video.path_mjpg = path_mjpg;
    video.path_mjpgload = path_mjpgload;

    // Get MicroSecPerFrame for MJPEG
    err = get_avi_info(&avi, file_mjpg);
    video.microsec_per_frame = err ? 0 : avi.dwMicroSecPerFrame;
    video.total_frames = avi.dwTotalFrames;

    // Read MJPG frame info
    fread(&mjpg_frame_start, 4, 1, file_mjpgload);
    fread(&mjpg_frame_size,  4, 1, file_mjpgload);

    if(mjpg_frame_size > JPG_BUF_SIZE) {
        printf("JPGBUF_SIZE is not enough.\n");
        err = JPG_ERR_NO_MEM;
        goto END;
    }

    // Read MJPG frame data from SD card
    fseek(file_mjpg, mjpg_frame_start, SEEK_SET); // set start point
    fread(jpg_buf, 1, mjpg_frame_size, file_mjpg); // read from set point

    // Parse MJPG picture header and get picture for user and decoder
    err = (jpg_err_t)jpeg_dec_parse_header(jpg_dec, &jpg_io, &jpg_info);
    if(err) goto END;

    // Set window size for image
    err = set_image_window(jpg_info.width, jpg_info.height, x0, y0);
    if (err) {
        err = JPG_ERR_IMAGE_SIZE_OVER;
        goto END;
    }

    // JPG test decode 
    #if JPG_BLOCK_DEC_ENABLE
        err = (jpg_err_t)jpeg_dec_get_outbuf_len(jpg_dec, &video.block_size);
        if(err) goto END;
        printf("Image block size: %d\n", video.block_size);

        err = (jpg_err_t)jpeg_dec_get_process_count(jpg_dec, &video.block_count); // Get process count
        if(err) goto END;
        printf("Image block count for 1 frame: %d\n", video.block_count);

        video.remain_size = (jpg_info.width * jpg_info.height * 2) % video.block_size;
        if (video.remain_size == 0) video.remain_size = video.block_size;

        jpg_io.outbuf = (uint8_t *)imgBuf[0]; // Set output buffer
        err = (jpg_err_t)jpeg_dec_process(jpg_dec, &jpg_io); // 1 block test decode 
    #else
        video.output_size = jpg_info.width * jpg_info.height * 2;
        
        jpg_io.outbuf = (uint8_t *)imgBuf; // Set output buffer
        err = (jpg_err_t)jpeg_dec_process(jpg_dec, &jpg_io); // Test decode
    #endif

    if(err)goto END; // Decode error
    
    // Close files
    fclose(file_mjpg);
    fclose(file_mjpgload);
        
    // Create Tasks
    xTaskCreatePinnedToCore(task_mjpg_decode, "MJPG_DECODE", 3072, &video, taskPriority, NULL, Core0or1);
	xSemaphoreTake(mjpg_dec_standby, portMAX_DELAY); // Wait for MJPG decode start

    return JPG_OK;

    END:
        // Close files
        if(file_mjpg) fclose(file_mjpg);
        if(file_mjpgload) fclose(file_mjpgload);

        if(func_end_mjpg) func_end_mjpg(&video);

        return err;
}


// x0, y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t start_mjpg(char *path_mjpg, int16_t x0, int16_t y0, const BaseType_t Core0or1, UBaseType_t taskPriority)
{
    jpg_err_t err = set_mjpg(path_mjpg, x0, y0, Core0or1, taskPriority);

    mjpg_start_command();

    return err;
}


// x0, y0 are offset value, but you set the value to negatve, the image is centerd.
jpg_err_t play_mjpg(char *path_mjpg, int16_t x0, int16_t y0)
{
    jpg_err_t err = start_mjpg(path_mjpg, x0, y0, 1, 20);  
    if(err) return err;

    xSemaphoreTake(mjpg_dec_end, portMAX_DELAY); // Wait for MJPG decode end
    
	if(!mjpgContinue) err = JPG_ERR_END_COMMAND;

    return err;
}
