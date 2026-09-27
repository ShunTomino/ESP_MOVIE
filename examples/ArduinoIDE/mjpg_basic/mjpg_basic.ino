// Basic Motion JPEG Player Sample for Arduino IDE
//
// 1. Change the user setting.(/ESP_MOVIE/src/USER_SETTING.h)
// 2. Copy the sample media files like AVI(MotionJPEG) to your SD card from ESP_MOVIE\examples\sample_media.
// 3. Compile and download this program to your hardware.
//
#include <USER_SETTING.h>
#include <movie_player.h>

void setup()
{ // put your setup code here, to run once:

  init_sd(); // SD Card Initialzation
  init_display(); // Display Initialzation
  init_backlight(); // Display Back Light Initialzation

  init_jpg_decoder(NULL); // MJPG(Motion JPEG) Decoder Initialzation
  set_dir_for_loadfiles(PATH_SD); // Set directory for generated files

  backlight_volume(170); // Set 0~255
}

void loop()
{ // put your main code here, to run repeatedly:

  display_color_all(RGB565_WHITE); // Change the display screen color

  play_mjpg("/sd/mjpg_160x96.avi", -1, -1); // Play MJPG
  //play_mjpg("/sd/mjpg_320x176.avi", -1, -1); // Play MJPG

  display_color_all(RGB565_BLACK); // Change the display screen color

  delay(3000); // Wait 3000 ms
}
