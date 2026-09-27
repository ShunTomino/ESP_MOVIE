// Basic MP3 Player Sample for Arduino IDE
//
// 1. Change the user setting.(/ESP_MOVIE/src/USER_SETTING.h)
// 2. Copy the sample media files like MP3 to your SD card from ESP_MOVIE\examples\sample_media.
// 3. Compile and download this program to your hardware.
//
#include <USER_SETTING.h>
#include <movie_player.h>

void setup()
{ // put your setup code here, to run once:

  init_sd(); // SD Card Initialzation
  init_i2s_dac(); // Audio DAC Initialzation

  init_mp3_decoder(NULL); // MP3 Decoder Initialzation

  set_audio_volume(5); // Set 0~20
}

void loop()
{ // put your main code here, to run repeatedly:

  play_mp3("/sd/audio.mp3"); // Play MP3

  delay(1000); // Wait 1000 ms
}
