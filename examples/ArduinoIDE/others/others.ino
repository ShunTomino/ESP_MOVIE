// Other Functions Sample for Arduino IDE
//
// 1. Change the user setting.(/ESP_MOVIE/src/USER_SETTING.h)
// 2. Copy the sample media files like JPEG to your SD card from ESP_MOVIE\examples\sample_media.
// 3. Compile and download this program to your hardware.
//
#include <USER_SETTING.h>
#include <movie_player.h>

void setup()
{ // put your setup code here, to run once:

  init_sd(); // SD Card Initialzation
  init_display(); // Display Initialzation
  init_backlight(); // Display Back Light Initialzation
  init_jpg_decoder(NULL); // JPEG Decoder Initialzation

  backlight_volume(170); // Set 0~255
}

void loop()
{ // put your main code here, to run repeatedly:
  jpeg_dec_header_info_t img_info;

  display_color_all(RGB565_BLACK); // Display the full screen color

  // Display JPEG image
  display_jpg("/sd/picture_160x96.jpg", -1, -1, &img_info);
  //display_jpg("/sd/picture_320x176.jpg", -1, -1, &img_info);
  //display_jpg("/sd/picture_320x240.jpg", -1, -1, &img_info);
  delay(3000); // Wait Time[ms]

  // Display rectangular areas
  display_color(RGB565_RED, 0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
  display_color(RGB565_CYAN, 20, 20, LCD_WIDTH-20-1, LCD_HEIGHT-20-1);
  display_color(RGB565_BLUE, 40, 40, LCD_WIDTH-40-1, LCD_HEIGHT-40-1);

  delay(1000); // Wait Time[ms]

  display_color_all(RGB565_GREEN); // Display the full screen color

  // Display 1 line Strings
  display_string("abcdefghijklmnopqrstuvwxyz", RGB565_BLACK, RGB565_WHITE, 0, 0);
  display_string("ABCDEFGHIJKLMNOPQRSTUVWXYZ", RGB565_CYAN, RGB565_MAGENTA, 40, 16);
  display_string("0123456789!?$&+-=~()[]{}/\:;,.", RGB565_RED, RGB565_YELLOW, 0, 32);

  delay(3000); // Wait Time[ms]
}
