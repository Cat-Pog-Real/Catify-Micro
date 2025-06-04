#ifndef HUD_H
#define HUD_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <cstdint>
#include <mymath.h>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>

void clear_screen(Adafruit_ILI9341 tft, uint16_t color);

void draw_rect(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color);

void draw_text(Adafruit_ILI9341 tft, iVector2 pos, uint16_t color, int size, const char c[]);

void draw_centered_text(Adafruit_ILI9341 tft, iVector2 pos, uint16_t color, int size, const char c[]);

int get_text_width(const String& text, int text_size);

std::vector<String> confine_text(const String& text, int text_size, iVector2 size, bool word_wrap);

class Box {
  public:
    iVector2 pos;
    iVector2 size;
    uint16_t color;
    Box(iVector2 pos, iVector2 size, uint16_t color);
    void draw(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color);
};

class Button {
  public:
    iVector2 pos;
    iVector2 size;
    uint16_t color;
    String text;
    int text_size;
    iVector2 text_buffer;
    uint16_t text_color;
    std::function<void(int)> func;
    int arg;
    Button(iVector2 pos, iVector2 size, uint16_t color, String text, int text_size, iVector2 text_buffer, uint16_t text_color, std::function<void(int)> func, int arg);
    void update(iVector2 point, iVector2 pos, iVector2 size, std::function<void(int)> func, int arg);
    void draw(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color, String text, int text_size, uint16_t text_color, iVector2 text_buffer);
};

class Songbox {
  public:
    iVector2 pos;
    iVector2 size;
    uint16_t background_color;
    iVector2 hit_buffer;
    iVector2 hit_size;
    uint16_t hit_color;
    String song_path;
    String song_name;
    int song_index;
    int text_size;
    iVector2 text_buffer;
    uint16_t text_color;
    Songbox(iVector2 pos, iVector2 size, uint16_t background_color, iVector2 hit_buffer, iVector2 hit_size, uint16_t hit_color, String song_path, String song_name, int song_index, int text_size, iVector2 text_buffer, uint16_t text_color);
    void draw(int song_selected, Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t background_color, iVector2 hit_buffer, iVector2 hit_size, uint16_t hit_color, String song_name, int song_index, int text_size, iVector2 text_buffer, uint16_t text_color);
    void draw_hit(int song_selected, Adafruit_ILI9341 tft, iVector2 pos, iVector2 hit_buffer, iVector2 hit_size, uint16_t hit_color, int song_index);
    int update(iVector2 point, iVector2 pos, iVector2 size, int song_index);
};

class Page {
  public:
    int start_index;
    int end_index;
    Page(int start_index, int end_index);
};

class VSlider {
  public:
    iVector2 pos;
    iVector2 size;
    uint16_t color;
    uint16_t not_color;
    iVector2 selector_pos;
    iVector2 selector_size;
    uint16_t selector_color;
    float max_value;
    float min_value;
    float value;
    int buffer;
    VSlider(iVector2 pos, iVector2 size, uint16_t color, uint16_t not_color, iVector2 selector_pos, iVector2 selector_size, uint16_t selector_color, float max_value, float min_value, float value, int buffer);
    void draw(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color, uint16_t not_color, iVector2 selector_pos, iVector2 selector_size, uint16_t selector_color);
    float update(iVector2 point, Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color, uint16_t not_color, iVector2 selector_pos, iVector2 selector_size, uint16_t selector_color, float max_value, float min_value, float value, int buffer, uint16_t background_color);
};

#endif