// ESP_MOVIE - Motion JPEG and MP3 player library for ESP32 Series
// Original source: https://github.com/ShunTomino/ESP_MOVIE.git
#include "movie_player.h"


movie_err_t init_movie_player(FuncEndMJPG func_end_MJPG, FuncEndMP3 func_end_MP3)
{
    int16_t err;

    init_mp3_decoder(func_end_MP3);

    err = init_jpg_decoder(func_end_MJPG);
    if(err) return (movie_err_t)err;

    return MOVIE_OK;
}


movie_err_t deinit_movie_player()
{
    int16_t err;

    deinit_mp3_decoder();

    err = deinit_jpg_decoder();
    if(err) return (movie_err_t)err;

    return MOVIE_OK;
}


// Set path of directory for mp3load file and mjpg load file
void set_dir_for_loadfiles(const char *path_directory)
{
    set_dir_for_mjpgload(path_mjpgload, path_directory);
}


void movie_start_command()
{
    mp3_start_command();
    mjpg_start_command();
}


void movie_pause_command()
{
    mp3_pause_command();
    mjpg_pause_command();
}


void movie_end_command()
{
    mp3_end_command();
    mjpg_end_command();
}


// x0, y0 are offset value, but you set the value to negative, the image is centered.
movie_err_t start_movie(char *path_mjpg, char *path_mp3, int16_t x0, int16_t y0, UBaseType_t taskPriority)
{
    jpg_err_t err_mjpg = set_mjpg(path_mjpg, x0, y0, 1, taskPriority);
    mp3_err_t err_mp3  = set_mp3(path_mp3, 0, taskPriority);
    
    movie_start_command();
    
    if(err_mjpg) return (movie_err_t)err_mjpg;
    if(err_mp3) return (movie_err_t)(err_mp3 + MP3_ERR_OFFSET);

    return MOVIE_OK;
}


// x0, y0 are offset value, but you set the value to negative, the image is centered.
movie_err_t play_movie(char *path_mjpg, char *path_mp3, int16_t x0, int16_t y0)
{
    movie_err_t err = start_movie(path_mjpg, path_mp3, x0, y0, 20);
    if(err) return err;

    xSemaphoreTake(mjpg_dec_end, portMAX_DELAY); // Wait for MJPG decode end
	xSemaphoreTake(mp3_dec_end, portMAX_DELAY);  // Wait for MP3 decode end

    if(!mjpgContinue & !mp3Continue) err = MOVIE_END_COMMAND;
    else if(!mjpgContinue) err = MOVIE_MJPG_END_COMMAND;
    else if(!mp3Continue) err = MOVIE_MP3_END_COMMAND;

    return err;
}
