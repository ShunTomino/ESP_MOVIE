// Basic Movie Player Sample for Arduino IDE
//
// 1. Change the user setting.(/ESP_MOVIE/src/USER_SETTING.h)
// 2. Copy the sample media files like MP3, AVI(MotionJPEG) to your SD card from ESP_MOVIE\examples\sample_media.
// 3. Compile and download this program to your hardware.
//
#include <USER_SETTING.h>
#include <movie_player.h>

void setup()
{ // put your setup code here, to run once:

  init_sd(); // SD Card Initialzation
  init_display(); // Display Initialzation
  init_backlight(); // Display Back Light Initialzation
  init_i2s_dac(); // Audio DAC Initialzation

  init_movie_player(NULL, NULL); // MJPG & MP3 Decoder Initialzation
  set_dir_for_loadfiles(PATH_SD); // Set directory for generated files

  set_audio_volume(5); // Set 0~20
  backlight_volume(170); // Set 0~255
}

void loop()
{ // put your main code here, to run repeatedly:

  display_color_all(RGB565_WHITE); // Change the display screen color

  play_movie("/sd/mjpg_160x96.avi", "/sd/audio.mp3", -1, -1); // Play MotionJPEG & MP3
  //play_movie("/sd/mjpg_320x176.avi", "/sd/audio.mp3", -1, -1); // Play MotionJPEG & MP3

  display_color_all(RGB565_BLACK); // Change the display screen color

  delay(3000); // Wait 3000 ms
}
