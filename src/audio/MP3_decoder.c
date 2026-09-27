// MP3 Decoder for ESP_MOVIE library
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "MP3_decoder.h"


// Buffer for audio output
int16_t pcmBuf[2304]; // PCM Output Buffer 1152 sample * 2ch(L ch & R ch)

// Buffer for MP3 frame data read
#if MP3_BUF_ENABLE_PSRAM
	EXT_RAM_BSS_ATTR uint8_t mp3Buf[MP3_BUF_SIZE];
#else
	uint8_t mp3Buf[MP3_BUF_SIZE];
#endif

// MP3 control
int16_t volumeTable[MAX_VOLUME + 1];
volatile int8_t mp3Volume = 3; // Min:0, Max:MAX_VOLUME

SemaphoreHandle_t mp3_dec_standby; // Semaphore for decode standby
SemaphoreHandle_t mp3_dec_start; // Semaphore for decode start
SemaphoreHandle_t mp3_dec_end; // Semaphore for decode end

bool mp3Continue = true; // Continue command
bool mp3Pause = false; // Pause command

// i2s DAC config
i2s_chan_handle_t i2s_handle = NULL;

HMP3Decoder mp3Dec;
MP3FrameInfo mp3Info;


// MP3バックグラウンド再生終了時のコールバック関数
static FuncEndMP3 func_end_mp3 = NULL;


// I2S DAC Initialization
esp_err_t init_i2s_dac()
{
	#define AUDIO_SAMPLE_RATE 44100 // Default: 44.1kHz
	esp_err_t err;

	i2s_chan_config_t i2s_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT_NUM, I2S_ROLE_MASTER);
	// dma_frame_size(4byte)*dma_frame_num*dma_desc_num = MP3 output sample(2304*2byte) * 2 buffer
	i2s_chan_cfg.dma_frame_num = 384;
	i2s_chan_cfg.dma_desc_num = 6;

	err = i2s_new_channel(&i2s_chan_cfg, &i2s_handle, NULL);
	if(err) return err;

	i2s_std_config_t i2s_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE),
		.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
		.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED,
			.bclk = PIN_I2S_BCLK,
			.ws = PIN_I2S_LRC,
			.dout = PIN_I2S_DOUT,
			.din = I2S_GPIO_UNUSED,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv = false,
			},
		},
	};
	
	// Initialize the channel
	err = i2s_channel_init_std_mode(i2s_handle, &i2s_cfg);
	if(err) return err;

	// Before writing data, start the TX channel first
	err = i2s_channel_enable(i2s_handle);
	if(err) return err;

	return ESP_OK;
}


esp_err_t deinit_i2s_dac(){
	esp_err_t err;

	err = i2s_channel_disable(i2s_handle);
	if(err) return err;

	err = i2s_del_channel(i2s_handle);
	if(err) return err;

	return ESP_OK;
}


// Change Output Sample Rate of I2S DAC
esp_err_t change_audio_output_sampleRate(int audio_samplerate)
{
	i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(audio_samplerate);
	esp_err_t err;

	err = i2s_channel_disable(i2s_handle);
	if(err) return err;

	err = i2s_channel_reconfig_std_clock(i2s_handle, &clk_cfg);
	if(err) return err;

	err = i2s_channel_enable(i2s_handle);
	if(err) return err;

	return ESP_OK;
}


void make_volume_table()
{
	float volume_curve[MAX_VOLUME + 1] = {0, 0.002, 0.005, 0.007, 0.01, 0.015, 0.02,
											0.025, 0.03, 0.05, 0.07, 0.1, 0.15, 0.2,
											0.25, 0.4, 0.5, 0.6, 0.8, 0.9, 1}; // 0~20

	int16_t i;
	for (i = 0; i < (MAX_VOLUME + 1); i++)
	{
		// volumeTable[i] = (32767 * i) / VOLUME_RANGE; // int16_t:-32768~32767
		volumeTable[i] = 32767 * volume_curve[i]; // int16_t:-32768~32767
	}
}


void set_audio_volume(int16_t volume)
{
	if (volume > MAX_VOLUME) mp3Volume = MAX_VOLUME;
	else if (volume < 0) mp3Volume = 0;
	else mp3Volume = volume;
}


// 固定小数点計算による音量調整
void apply_audio_volume(int16_t *audioBuf)
{
	int32_t audioSample;
	uint16_t i;

	for (i = 0; i < 2304; i++)
	{
		// audioBuf[i] = audioBuf[i]*0.005*volume;//*(volume/(VOLUME_RANGE-1));

		audioSample = (int32_t)audioBuf[i] * volumeTable[mp3Volume];

		audioSample >>= 15;

		if (audioSample > 32767)
		{
			audioSample = 32767 - ((audioSample - 32767) >> 2);
		}
		else if (audioSample < -32768)
		{
			audioSample = -32768 + ((-32768 - audioSample) >> 2);
		}
		audioBuf[i] = (int16_t)audioSample;
	}
}


void audio_silence()
{
	size_t writtenSize;

	memset(pcmBuf, 0, 4608);
	i2s_channel_write(i2s_handle, pcmBuf, 4608, &writtenSize, portMAX_DELAY);
	i2s_channel_write(i2s_handle, pcmBuf, 4608, &writtenSize, portMAX_DELAY);
}


#if (PIN_AUDIO_MUTE != -1)
	void audio_mute(){
		gpio_set_level(PIN_AUDIO_MUTE, 0);
	}

	void audio_unmute(){
		gpio_set_level(PIN_AUDIO_MUTE, 1);
	}
#endif


void init_mp3_decoder(FuncEndMP3 func_end)
{
	//audio_silence();
	make_volume_table();

	mp3Dec = MP3InitDecoder();

	#if (PIN_AUDIO_MUTE != -1)
		// Audio Mute Pin Setting
		gpio_reset_pin(PIN_AUDIO_MUTE);
		gpio_set_direction(PIN_AUDIO_MUTE, GPIO_MODE_OUTPUT);
		audio_unmute();
	#endif

	// CallBack Registration
	func_end_mp3 = func_end;

	mp3_dec_start = xSemaphoreCreateBinary();
}


void deinit_mp3_decoder()
{
	MP3FreeDecoder(mp3Dec);
}


void mp3_start_command()
{
	mp3Continue = true;
	mp3Pause = false;
    xSemaphoreGive(mp3_dec_start); // Decode start
}


void mp3_pause_command()
{
	mp3Continue = false;
	mp3Pause = true;
}


void mp3_end_command()
{
	mp3Continue = false;
	mp3Pause = false;
}


void task_mp3_decode(void *audio_info)
{
	audio_info_t audio = *(audio_info_t *)audio_info;
	FILE *file_mp3 = NULL;
	uint8_t *read_ptr = mp3Buf; // ring buffer current pointer
	uint8_t *read_ptr_pre;
	uint8_t tag_size[4]; // Skip data size bit
	uint32_t skip_size;
	size_t written_size;
	int ring_buf_size = 0; // Ring buffer readable size
	int bytes_pre;
	int offset, err;
	
	file_mp3 = fopen(audio.path_mp3, "rb");
	if (!file_mp3) goto END;

	fseek(file_mp3, 6, SEEK_SET); // set start point

	fread(tag_size, 1, 4, file_mp3);
	skip_size = (tag_size[0] << 21) | (tag_size[1] << 14) | (tag_size[2] << 7) | (tag_size[3]);

	fseek(file_mp3, 10 + skip_size, SEEK_SET); // set start point

	// Initialization Loop for setting Sample Rate
	while (1)
	{
		// Ring buffer read
		memmove(mp3Buf, read_ptr, ring_buf_size); // 残ったデータを先頭にコピー
		ring_buf_size = fread(mp3Buf + ring_buf_size, 1, MP3_BUF_SIZE - ring_buf_size, file_mp3) + ring_buf_size; // 残ったデータの後ろに新たなデータをつなげる
		read_ptr = mp3Buf; // ポインタを先頭に戻す

		if (ring_buf_size <= 0) break;

		while (ring_buf_size > 0)
		{
			offset = MP3FindSyncWord(read_ptr, ring_buf_size); // バッファ内でMP3フレームの先頭を返す
			if (offset < 0) break; // オフセットが見つからなかった場合

			read_ptr += offset;
			ring_buf_size -= offset;

			// save readPtr and bytes before decode
			read_ptr_pre = read_ptr;
			bytes_pre = ring_buf_size;

			// MP3Decode(mp3dec, &mp3Buf_psram, &mp3size, pcmBuf, 0);
			err = MP3Decode(mp3Dec, &read_ptr, &ring_buf_size, pcmBuf, 0); // MP3デコード
			if (err == ERR_MP3_NONE){
				MP3GetLastFrameInfo(mp3Dec, &mp3Info); // フレーム情報を取得
				audio.output_size = mp3Info.outputSamps * 2;
				change_audio_output_sampleRate(mp3Info.samprate); // Output Sample Rate Setting
				break;
			} else if (err == ERR_MP3_INDATA_UNDERFLOW) { // ファイル終端
				read_ptr = read_ptr_pre; // decoderが進めてしまったポインタをもとに戻す
				ring_buf_size = bytes_pre; // decoderが進めてしまったポインタをもとに戻す
				break;
			} else continue; // 次のフレームへ
		}
		if (mp3Info.samprate > 0) break;
	}

	// Reset
	read_ptr = mp3Buf;
	ring_buf_size = 0;
	fseek(file_mp3, 10 + skip_size, SEEK_SET); // set start point

	xSemaphoreGive(mp3_dec_standby); // Notice MJPG decode standby
	START:
	xSemaphoreTake(mp3_dec_start, portMAX_DELAY); // Wait for MP3 decode start

	// Normal Loop
	while (1) {
		// Ring buffer read
		memmove(mp3Buf, read_ptr, ring_buf_size); // 残ったデータを先頭にコピー
		ring_buf_size = fread(mp3Buf + ring_buf_size, 1, MP3_BUF_SIZE - ring_buf_size, file_mp3) + ring_buf_size; // 残ったデータの後ろに新たなデータをつなげる
		read_ptr = mp3Buf; // ポインタを先頭に戻す

		if ((ring_buf_size <= 0) | (mp3Continue ^ 1))break; // Finish

		while (ring_buf_size > 0) {
			offset = MP3FindSyncWord(read_ptr, ring_buf_size); // バッファ内でMP3フレームの先頭を返す
			if (offset < 0)	break; // オフセットが見つからなかった場合

			read_ptr += offset;
			ring_buf_size -= offset;

			// save readPtr and bytes before decode
			read_ptr_pre = read_ptr;
			bytes_pre = ring_buf_size;

			// MP3 Decode
			err = MP3Decode(mp3Dec, &read_ptr, &ring_buf_size, pcmBuf, 0);
			if (err == ERR_MP3_NONE) {
				apply_audio_volume(pcmBuf);
				i2s_channel_write(i2s_handle, pcmBuf, audio.output_size, &written_size, portMAX_DELAY);
			} else if (err == ERR_MP3_INDATA_UNDERFLOW) { // データ不足
				read_ptr = read_ptr_pre; // decoderが進めてしまったポインタをもとに戻す
				ring_buf_size = bytes_pre; // decoderが進めてしまったポインタをもとに戻す
				break;
			} else continue; // Frame skip
		}
	}

	END:
		audio_silence(); // Audio mute
		if (mp3Pause) goto START; // Pause

		if(file_mp3) fclose(file_mp3); // Close file

		if(func_end_mp3) func_end_mp3(&audio);
		xSemaphoreGive(mp3_dec_end); // Notice MJPG decode end

		printf("MP3 Task Remaining Stack: %u words\n", uxTaskGetStackHighWaterMark(NULL));
		vTaskDelete(NULL); // delete this task
}


mp3_err_t set_mp3(char *path_mp3, const BaseType_t Core0or1, UBaseType_t taskPriority)
{
	audio_info_t audio;
	FILE *file_mp3 = NULL;
	mp3_err_t err;

	audio.path_mp3 = path_mp3;
	audio.output_size = 0;
	
	// Semaphore Initialization
    mp3_dec_standby = xSemaphoreCreateBinary();
	mp3_dec_start = xSemaphoreCreateBinary();
	mp3_dec_end = xSemaphoreCreateBinary();

	// Open MP3 file
	file_mp3 = fopen(path_mp3, "rb");
	if (!file_mp3){
        printf("Open MP3 file error: %s\n", path_mp3);
        printf("SDK Config FF_MAX_LFN: %d\n", FF_MAX_LFN);
		err = MP3_ERR_OPEN_FILE;
		goto END;
	}

	fclose(file_mp3);

	// Create Task
    xTaskCreatePinnedToCore(task_mp3_decode, "MP3_DECODE", 3072, &audio, taskPriority, NULL, Core0or1);
	xSemaphoreTake(mp3_dec_standby, portMAX_DELAY); // Wait for MP3 decode standby

    return MP3_OK;

	END:
        // Close files
        if(file_mp3) fclose(file_mp3);

        if(func_end_mp3) func_end_mp3(&audio);

        return err;
}


mp3_err_t start_mp3(char *path_mp3, const BaseType_t Core0or1, UBaseType_t taskPriority)
{
	mp3_err_t err = set_mp3(path_mp3, Core0or1, taskPriority);

	mp3_start_command();

	return err;
}


mp3_err_t play_mp3(char *path_mp3)
{
	mp3_err_t err = start_mp3(path_mp3, 1, 20);
    if(err) return err;

	xSemaphoreTake(mp3_dec_end, portMAX_DELAY); // Wait for MP3 decode end

	if(!mp3Continue) err = MP3_ERR_END_COMMAND;

	return err;
}
