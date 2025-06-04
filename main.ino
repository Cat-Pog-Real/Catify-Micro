// Outside Libs
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <bb_captouch.h>
#include <vector>
#include <bits/stdc++.h>
#include <ArduinoJson.h>
#include <cstdlib>
#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"

// My Libs
#include <mymath.h>
#include "hud.h"

// Define Display Pins
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  0
#define TFT_SCK  14
#define TFT_MOSI 13
#define BL_PIN   27

// Define Touch Pins
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_RST 25
#define TOUCH_INT 21

// Define SD Pins
#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

// Define Speak Pins
#define SPEAKER_PIN 26

// Define DAC Pins
#define LCK 16
#define DIN 4
#define BCK 17

// Create a custom SPI instance
SPIClass mySPI(VSPI);  // Use VSPI or HSPI

// Touch reference
BBCapTouch touch;
TOUCHINFO ti;

// Initialize display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Create Audio object
Audio audio;

// Vars
const int DISPLAY_WIDTH = 240;
const int DISPLAY_HEIGHT = 320;
const int CENTER_X = 120;
const int CENTER_Y = 160;
int curScene = 1;

// Finger Vars
int num_fingers = 0;
int finger_x = 0;
int finger_y = 0;
const int NONE_FINGER = 0;
const int TAP_FINGER = 1;
const int HOLD_FINGER = 2;
const int DRAG_FINGER = 3;
int current_finger_state = NONE_FINGER;
int old_num_fingers = 0;
iVector2 tap_finger_pos = iVector2{0,0};
const int MAX_FINGER_RAD = 128;

// Song Vars
std::vector<String> songNames;
std::unordered_map<int, Songbox> songbox_dict;
int song_selected = -1;
String song_selected_name;
bool isPlaying = false;
int song_time_elapsed = -1;
bool repeat = false;
bool shuffle_songs = false;
std::vector<int> shuffleSongIndexes;
int song_selected_list[5] = {-1, -1, -1, -1, -1};
int song_selected_pos = 2;
const float STARTING_VOLUME = 32.0;

// Page Vars
std::map<int, Page> page_dict;
int page_selected = 0;

// FPS Vars
unsigned long lastFrameTime = 0;
float deltaTime = 0.0f;
float fps = 0.0f;
const int targetFPS = 40;
const int frameDuration = 1000 / targetFPS;  // in milliseconds

// Screen saver Vars
bool save_mode = true;
float time_without_input = 0.0;
const float MAX_TIME_WITHOUT_INPUT = 60.0;
bool screen_state = true;

// Error Vars
bool loaded_sd = true;
bool touch_found = true;

struct colors{
    uint16_t background = tft.color565(20,20,20);
    uint16_t black = tft.color565(0,0,0);
    uint16_t white = tft.color565(255,255,255);
    uint16_t grey = tft.color565(40,40,40);
    uint16_t red = tft.color565(255,0,0);
    uint16_t green = tft.color565(0,255,0);
    uint16_t blue = tft.color565(0,0,255);
    uint16_t catify_blue = tft.color565(101,163,191);
} color;

void load_songpage(int arg);
void load_homepage(int arg);
void change_page(int amount);
void media_toggle_function(int arg);
void media_skip_foward_function(int arg);
void media_skip_backward_function(int arg);
void media_toggle_shuffle_function(int arg);
void media_toggle_repeat_function(int arg);

Box top_panel(iVector2{0,0}, iVector2{240,34}, color.grey);
Box bottom_panel(iVector2{0,308}, iVector2{240,12}, color.grey);

Button Home_Page(iVector2{4,2}, iVector2{114,30}, color.catify_blue, "Home Page", 1, iVector2{57,15}, color.white, load_homepage, 100);
Button Song_Page(iVector2{122,2}, iVector2{114,30}, color.catify_blue, "Songs List", 1, iVector2{57,15}, color.white, load_songpage, 100);
Button skip_back_3(iVector2{210,36}, iVector2{28,43}, color.catify_blue, "-3", 1, iVector2{14,21}, color.white, change_page, -3);
Button skip_back_2(iVector2{210,81}, iVector2{28,43}, color.catify_blue, "-2", 1, iVector2{14,21}, color.white, change_page, -2);
Button skip_back_1(iVector2{210,126}, iVector2{28,43}, color.catify_blue, "-1", 1, iVector2{14,21}, color.white, change_page, -1);
Button skip_foward_1(iVector2{210,171}, iVector2{28,43}, color.catify_blue, "+1", 1, iVector2{14,21}, color.white, change_page, 1);
Button skip_foward_2(iVector2{210,216}, iVector2{28,43}, color.catify_blue, "+2", 1, iVector2{14,21}, color.white, change_page, 2);
Button skip_foward_3(iVector2{210,261}, iVector2{28,43}, color.catify_blue, "+3", 1, iVector2{14,21}, color.white, change_page, 3);
Button media_skip_backwards(iVector2{6, 242}, iVector2{72,30}, color.catify_blue, "Skip -1", 1, iVector2{36,15}, color.white, media_skip_backward_function, 100);
Button media_toggle(iVector2{84, 242}, iVector2{72,30}, color.catify_blue, "Play", 1, iVector2{36,15}, color.white, media_toggle_function, 100);
Button media_skip_foward(iVector2{162, 242}, iVector2{72,30}, color.catify_blue, "Skip +1", 1, iVector2{36,15}, color.white, media_skip_foward_function, 100);
Button media_toggle_shuffle(iVector2{126, 276}, iVector2{30,30}, color.grey, "Shuffle:", 1, iVector2{-27,15}, color.white, media_toggle_shuffle_function, 100);
Button media_toggle_repeat(iVector2{204, 276}, iVector2{30,30}, color.grey, "Repeat:", 1, iVector2{-22,15}, color.white, media_toggle_repeat_function, 100);
VSlider VolumeSlider(iVector2{214, 45}, iVector2{22, 168}, color.catify_blue, color.grey, iVector2{213,100}, iVector2{24,18}, color.white, 100.0, 0.0, STARTING_VOLUME, 15);

void load_songpage(int arg) {
  curScene = 1;
  draw_rect(tft, iVector2{0, top_panel.size.y}, iVector2{240,286}, color.background);
  top_panel.draw(tft, top_panel.pos, top_panel.size, top_panel.color);
  load_page(page_selected);
  Home_Page.draw(tft, Home_Page.pos, Home_Page.size, Home_Page.color, Home_Page.text, Home_Page.text_size, Home_Page.text_color, Home_Page.text_buffer);
  skip_back_3.draw(tft, skip_back_3.pos, skip_back_3.size, skip_back_3.color, skip_back_3.text, skip_back_3.text_size, skip_back_3.text_color, skip_back_3.text_buffer);
  skip_back_2.draw(tft, skip_back_2.pos, skip_back_2.size, skip_back_2.color, skip_back_2.text, skip_back_2.text_size, skip_back_2.text_color, skip_back_2.text_buffer);
  skip_back_1.draw(tft, skip_back_1.pos, skip_back_1.size, skip_back_1.color, skip_back_1.text, skip_back_1.text_size, skip_back_1.text_color, skip_back_1.text_buffer);
  skip_foward_1.draw(tft, skip_foward_1.pos, skip_foward_1.size, skip_foward_1.color, skip_foward_1.text, skip_foward_1.text_size, skip_foward_1.text_color, skip_foward_1.text_buffer);
  skip_foward_2.draw(tft, skip_foward_2.pos, skip_foward_2.size, skip_foward_2.color, skip_foward_2.text, skip_foward_2.text_size, skip_foward_2.text_color, skip_foward_2.text_buffer);
  skip_foward_3.draw(tft, skip_foward_3.pos, skip_foward_3.size, skip_foward_3.color, skip_foward_3.text, skip_foward_3.text_size, skip_foward_3.text_color, skip_foward_3.text_buffer);
  draw_centered_text(tft, iVector2{179,17}, color.white, 1, "Catify-Micro");
  bottom_panel.draw(tft, bottom_panel.pos, bottom_panel.size, bottom_panel.color);
  if (song_selected != -1) {
    Songbox last_songbox = songbox_dict.at(song_selected);
    song_selected_name = last_songbox.song_name;
    std::vector<String> new_text = confine_text(song_selected_name, 1, iVector2{200, 8}, false);
    const char *c = new_text.at(0).c_str();
    draw_text(tft, iVector2{2,310}, color.white, 1, c);
  }
}

void load_homepage(int arg) {
  curScene = 0;
  draw_rect(tft, iVector2{0, top_panel.pos.y}, iVector2{240,320}, color.background);
  top_panel.draw(tft, top_panel.pos, top_panel.size, top_panel.color);
  Song_Page.draw(tft, Song_Page.pos, Song_Page.size, Song_Page.color, Song_Page.text, Song_Page.text_size, Song_Page.text_color, Song_Page.text_buffer);
  draw_centered_text(tft, iVector2{61,17}, color.white, 1, "Catify-Micro");
  if (song_selected != -1) {
    String img_path = get_raw_path(songNames, song_selected);
    draw_raw(tft, img_path, iVector2{30, 39});
    std::vector<String> new_text = confine_text(song_selected_name, 1, iVector2{230, 8}, false);
    const char *c = new_text.at(0).c_str();
    draw_centered_text(tft, iVector2{CENTER_X,230}, color.white, 1, c);
    media_skip_backwards.draw(tft, media_skip_backwards.pos, media_skip_backwards.size, media_skip_backwards.color, media_skip_backwards.text, media_skip_backwards.text_size, media_skip_backwards.text_color, media_skip_backwards.text_buffer);
    if (audio.isRunning() && isPlaying == true) {
      media_toggle.text = "Pause";
    } else {
      media_toggle.text = "Play";
    }
    media_toggle.draw(tft, media_toggle.pos, media_toggle.size, media_toggle.color, media_toggle.text, media_toggle.text_size, media_toggle.text_color, media_toggle.text_buffer);
    media_skip_foward.draw(tft, media_skip_foward.pos, media_skip_foward.size, media_skip_foward.color, media_skip_foward.text, media_skip_foward.text_size, media_skip_foward.text_color, media_skip_foward.text_buffer);
    media_toggle_shuffle.draw(tft, media_toggle_shuffle.pos, media_toggle_shuffle.size, media_toggle_shuffle.color, media_toggle_shuffle.text, media_toggle_shuffle.text_size, media_toggle_shuffle.text_color, media_toggle_shuffle.text_buffer);
    media_toggle_repeat.draw(tft, media_toggle_repeat.pos, media_toggle_repeat.size, media_toggle_repeat.color, media_toggle_repeat.text, media_toggle_repeat.text_size, media_toggle_repeat.text_color, media_toggle_repeat.text_buffer);
    VolumeSlider.draw(tft, VolumeSlider.pos, VolumeSlider.size, VolumeSlider.color, VolumeSlider.not_color, VolumeSlider.selector_pos, VolumeSlider.selector_size, VolumeSlider.selector_color);
  } else {
    draw_centered_text(tft, iVector2{CENTER_X,177}, color.white, 1, "Select a Song.");
  }
}

void change_page(int amount) {
  page_selected += amount;
  int num_pages = page_dict.size()-1;
  if (page_selected < 0) {
    page_selected = (num_pages-abs(page_selected))+1;
  }
  if (page_selected > num_pages) {
    page_selected = (num_pages-page_selected)+amount+(amount-1);
  }
  load_page(page_selected);
}

void media_toggle_function(int arg) {
  toggle_song();
  if (audio.isRunning() && isPlaying == true) {
    media_toggle.text = "Pause";
  } else {
    media_toggle.text = "Play";
  }
  media_toggle.draw(tft, media_toggle.pos, media_toggle.size, media_toggle.color, media_toggle.text, media_toggle.text_size, media_toggle.text_color, media_toggle.text_buffer);
}

void update_hud() {
  if (curScene == 0) {
    Songbox last_songbox = songbox_dict.at(song_selected);
    song_selected_name = last_songbox.song_name;
    std::vector<String> new_text = confine_text(song_selected_name, 1, iVector2{230, 8}, false);
    const char *c = new_text.at(0).c_str();
    draw_rect(tft, iVector2{0, 226}, iVector2{240, 8}, color.background);
    draw_centered_text(tft, iVector2{CENTER_X,230}, color.white, 1, c);
    String img_path = get_raw_path(songNames, song_selected);
    draw_raw(tft, img_path, iVector2{30, 39});
    if (audio.isRunning() && isPlaying == true) {
      media_toggle.text = "Pause";
    } else {
      media_toggle.text = "Play";
    }
    media_toggle.draw(tft, media_toggle.pos, media_toggle.size, media_toggle.color, media_toggle.text, media_toggle.text_size, media_toggle.text_color, media_toggle.text_buffer);
  } else if (curScene == 1) {
    Songbox last_songbox = songbox_dict.at(song_selected);
    song_selected_name = last_songbox.song_name;
    std::vector<String> new_text = confine_text(song_selected_name, 1, iVector2{200, 8}, false);
    const char *c = new_text.at(0).c_str();
    draw_text(tft, iVector2{2,310}, color.white, 1, c);
  }
  if (audio.isRunning() == false) {
    Serial.println(song_selected);
    Serial.println(song_selected_name);
  }
}

void media_skip_foward_function(int arg) {
  skip_foward();
  update_hud();
}
void media_skip_backward_function(int arg) {
  skip_backward();
  update_hud();
}

void media_toggle_shuffle_function(int arg) {
  if (shuffle_songs == true) {
    shuffle_songs = false;
    media_toggle_shuffle.color = color.grey;
  } else {
    shuffle_songs = true;
    media_toggle_shuffle.color = color.catify_blue;
  }
  if (repeat == true) {
    repeat = false;
    media_toggle_repeat.color = color.grey;
    media_toggle_repeat.draw(tft, media_toggle_repeat.pos, media_toggle_repeat.size, media_toggle_repeat.color, media_toggle_repeat.text, media_toggle_repeat.text_size, media_toggle_repeat.text_color, media_toggle_repeat.text_buffer);
  }
  media_toggle_shuffle.draw(tft, media_toggle_shuffle.pos, media_toggle_shuffle.size, media_toggle_shuffle.color, media_toggle_shuffle.text, media_toggle_shuffle.text_size, media_toggle_shuffle.text_color, media_toggle_shuffle.text_buffer);
}

void media_toggle_repeat_function(int arg) {
  if (repeat == true) {
    repeat = false;
    media_toggle_repeat.color = color.grey;
  } else {
    repeat = true;
    media_toggle_repeat.color = color.catify_blue;
  }
  if (shuffle_songs == true) {
    shuffle_songs = false;
    media_toggle_shuffle.color = color.grey;
    media_toggle_shuffle.draw(tft, media_toggle_shuffle.pos, media_toggle_shuffle.size, media_toggle_shuffle.color, media_toggle_shuffle.text, media_toggle_shuffle.text_size, media_toggle_shuffle.text_color, media_toggle_shuffle.text_buffer);
  }
  media_toggle_repeat.draw(tft, media_toggle_repeat.pos, media_toggle_repeat.size, media_toggle_repeat.color, media_toggle_repeat.text, media_toggle_repeat.text_size, media_toggle_repeat.text_color, media_toggle_repeat.text_buffer);
}

void setup() {

  // Initialize SPI bus for microSD Card
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Initialize SPI bus for microSD Card
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);

  pinMode(BL_PIN, OUTPUT);
  digitalWrite(BL_PIN, HIGH);

  Serial.begin(115200);

  // Start microSD Card
  if(!SD.begin(SD_CS)) {
    Serial.println("Error accessing microSD card!");
    loaded_sd = false;
    //while(true);
  }

  Serial.println("Succesfully accessed microSD card!");

  //audio.setPinout(0, 0, SPEAKER_PIN); // BLCK, LRC, DOUT 
  //audio.setVolumeSteps(255); // max 255
  //audio.setVolume(5); // max ^^^
    
  // Open music file
  //const char song_path[] PROGMEM = "/songs/6 Dogs - Saturn (Audio)/6 Dogs - Saturn (Audio).mp3";
  //audio.connecttoFS(SD, "/songs/6 Dogs - Saturn (Audio)/6 Dogs - Saturn (Audio).mp3");
  //                      "/songs/6 Dogs - Saturn (Audio)/6 Dogs - Saturn (Audio).mp3"

  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  // Init touch controller
  int result = touch.init(TOUCH_CYD_24C);
  if (result == CT_SUCCESS) {
    Serial.println("Touch controller initialized.");
  } else {
    Serial.println("Touch controller NOT found!");
    touch_found = false;
  }

  mySPI.begin(TFT_SCK, -1, TFT_MOSI);

  tft.begin();

  tft.setRotation(2);
  touch.setOrientation(180, 240, 320);

  if (loaded_sd == true && touch_found == true) {
    clear_screen(tft, color.background);
    draw_centered_text(tft, iVector2{CENTER_X, CENTER_Y}, color.catify_blue, 1, "Loading Songs");

    //Serial.println(ESP.getFreeHeap());
    listSongNames();
    load_Songbox();

    clear_screen(tft, color.background);

    load_songpage(0);
    //Serial.println(ESP.getFreeHeap());

    //audio.setPinout(0, 0, SPEAKER_PIN); // BLCK, LRC, DOUT 
    audio.setPinout(BCK, LCK, DIN); // BLCK, LRC, DOUT 
    audio.setVolumeSteps(100); // max 255
    audio.setVolume((int)STARTING_VOLUME); // max ^^^
    //audio.setTone(-20, -20, -20);

    // audio.connecttoFS(SD, "/songs/6 Dogs - Saturn (Audio)/6 Dogs - Saturn (Audio).mp3");

    Serial.print("Succesful Boot! \n");
  } else if (loaded_sd == false && touch_found == true){
    clear_screen(tft, color.background);
    draw_centered_text(tft, iVector2{CENTER_X, CENTER_Y}, color.catify_blue, 1, "Couldn't find SD-Card!");
    while(1);
  } else if (loaded_sd == true && touch_found == false) {
    clear_screen(tft, color.background);
    draw_centered_text(tft, iVector2{CENTER_X, CENTER_Y}, color.catify_blue, 1, "Couldn't find Touch!");
    while(1);
  } else if (loaded_sd == false && touch_found == false) {
    clear_screen(tft, color.background);
    draw_centered_text(tft, iVector2{CENTER_X, CENTER_Y}, color.catify_blue, 1, "You're Porked...");
    while(1);
  }
}

void loop() {
  unsigned long currentTime = millis();
  deltaTime = (currentTime - lastFrameTime) / 1000.0f;  // seconds
  //fps = 1.0f / deltaTime;

  audio.loop();
  int current_time = audio.getAudioCurrentTime();
  if (song_time_elapsed < current_time && audio.isRunning() == true) {
    song_time_elapsed = current_time;
    //Serial.println(song_time_elapsed);
    if (isPlaying == true && song_time_elapsed >= 0 && curScene == 0 && screen_state == true) {
      int width = lerpf(0.0, 240.0, (float)song_time_elapsed/(float)audio.getAudioFileDuration());
      draw_rect(tft, iVector2{0,310}, iVector2{width,10}, color.white);
      draw_rect(tft, iVector2{width,310}, iVector2{DISPLAY_WIDTH-width,10}, color.grey);
      draw_rect(tft, iVector2{0,300}, iVector2{40,8}, color.background);
      int time_left = audio.getAudioFileDuration()-song_time_elapsed;
      int minutes_left = (int)((float)time_left / 60.0);
      int seconds_left = time_left % 60;
      String time_str;
      if (seconds_left < 10) {
        time_str = String(minutes_left) + ":0" + String(seconds_left);
      } else {
        time_str = String(minutes_left) + ":" + String(seconds_left);
      }
      const char *c = time_str.c_str();
      draw_text(tft, iVector2{2, 300}, color.white, 1, c);
    }
  }
  if (song_time_elapsed > -1 && audio.isRunning() == false && isPlaying == true) {
    isPlaying = false;
    if (repeat == true) {
      Serial.println("repeat song");
      Songbox songbox = songbox_dict.at(song_selected);
      const char *c = songbox.song_path.c_str();
      play_song(c);
    } else if (shuffle_songs == true) {
      Serial.println("random song");
      skip_foward();
      update_hud();
    }
  }

  handle_touch();

  if (save_mode == true) {
    if (num_fingers <= 0) {
      time_without_input += deltaTime;
      if (time_without_input >= MAX_TIME_WITHOUT_INPUT) {
        if (screen_state == true) {
          screen_state = false;
          digitalWrite(BL_PIN, LOW);
        }
      }
    } else {
      time_without_input = 0.0;
      if (screen_state == false) {
        screen_state = true;
        digitalWrite(BL_PIN, HIGH);
      }
    }
  }

  if (num_fingers > 0 && current_finger_state == TAP_FINGER && screen_state == true) {
    if (curScene == 0) {
      Song_Page.update(iVector2{finger_x, finger_y}, Song_Page.pos, Song_Page.size, Song_Page.func, Song_Page.arg);
      if (song_selected != -1) {
        media_skip_backwards.update(iVector2{finger_x, finger_y}, media_skip_backwards.pos, media_skip_backwards.size, media_skip_backwards.func, media_skip_backwards.arg);
        media_toggle.update(iVector2{finger_x, finger_y}, media_toggle.pos, media_toggle.size, media_toggle.func, media_toggle.arg);
        media_skip_foward.update(iVector2{finger_x, finger_y}, media_skip_foward.pos, media_skip_foward.size, media_skip_foward.func, media_skip_foward.arg);
        media_toggle_shuffle.update(iVector2{finger_x, finger_y}, media_toggle_shuffle.pos, media_toggle_shuffle.size, media_toggle_shuffle.func, media_toggle_shuffle.arg);
        media_toggle_repeat.update(iVector2{finger_x, finger_y}, media_toggle_repeat.pos, media_toggle_repeat.size, media_toggle_repeat.func, media_toggle_repeat.arg);
      }
    } else if (curScene == 1) {
      Home_Page.update(iVector2{finger_x, finger_y}, Home_Page.pos, Home_Page.size, Home_Page.func, Home_Page.arg);
      skip_back_3.update(iVector2{finger_x, finger_y}, skip_back_3.pos, skip_back_3.size, skip_back_3.func, skip_back_3.arg);
      skip_back_2.update(iVector2{finger_x, finger_y}, skip_back_2.pos, skip_back_2.size, skip_back_2.func, skip_back_2.arg);
      skip_back_1.update(iVector2{finger_x, finger_y}, skip_back_1.pos, skip_back_1.size, skip_back_1.func, skip_back_1.arg);
      skip_foward_1.update(iVector2{finger_x, finger_y}, skip_foward_1.pos, skip_foward_1.size, skip_foward_1.func, skip_foward_1.arg);
      skip_foward_2.update(iVector2{finger_x, finger_y}, skip_foward_2.pos, skip_foward_2.size, skip_foward_2.func, skip_foward_2.arg);
      skip_foward_3.update(iVector2{finger_x, finger_y}, skip_foward_3.pos, skip_foward_3.size, skip_foward_3.func, skip_foward_3.arg);
      update_page(page_selected, iVector2{finger_x, finger_y});
    }
  }
  if (num_fingers > 0 && screen_state == true && curScene == 0 && song_selected != -1) {
    float Volume = VolumeSlider.update(iVector2{finger_x, finger_y}, tft, VolumeSlider.pos, VolumeSlider.size, VolumeSlider.color, VolumeSlider.not_color, VolumeSlider.selector_pos, VolumeSlider.selector_size, VolumeSlider.selector_color, VolumeSlider.max_value, VolumeSlider.min_value, VolumeSlider.value, VolumeSlider.buffer, color.background);
    if (audio.getVolume() != (int)Volume) {
      audio.setVolume((int)Volume);
    }
    //if (int(Volume) > 0) {
    //  audio.setVolume(int(Volume));
    //} else {
    //  audio.setVolume(0);
    //}
  }

  // Cap framerate
  unsigned long elapsed = millis() - currentTime;
  if (elapsed < frameDuration) {
    delay(frameDuration - elapsed);
  }

  //String strfps = String(fps);
  //const char *c = strfps.c_str();
  //bottom_panel.draw(tft, bottom_panel.pos, bottom_panel.size, bottom_panel.color);
  //draw_centered_text(tft, iVector2{120, 316}, color.white, 1, c);

  lastFrameTime = currentTime;
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}