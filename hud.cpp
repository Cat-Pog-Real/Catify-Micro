#include "hud.h"

const iVector2 default_font_size = iVector2{6, 8};

void clear_screen(Adafruit_ILI9341 tft, uint16_t color) {
  tft.fillScreen(color);
}

void draw_rect(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color) {
  tft.fillRect(pos.x, pos.y, size.x, size.y, color);
}

void draw_text(Adafruit_ILI9341 tft, iVector2 pos, uint16_t color, int size, const char c[]) {
  tft.setCursor(pos.x, pos.y);
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.println(c);
}

void draw_centered_text(Adafruit_ILI9341 tft, iVector2 pos, uint16_t color, int size, const char c[]) {
  size_t length = strlen(c);
  int word_width = default_font_size.x*size*length;
  tft.setCursor(pos.x-(int)(word_width/2), pos.y-((int)((default_font_size.y*size)/2)));
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.println(c);
}

int get_text_width(const String& text, int text_size) {
    return static_cast<int>(text.length()) * 6 * text_size;
}

// Get substring from `str`, starting at `start`, up to `length` characters
String substring(const String& str, int start, int length) {
    String result = "";
    for (int i = 0; i < length && (start + i) < (int)str.length(); ++i) {
        result += str[start + i];
    }
    return result;
}

// Find last space in the string; return -1 if not found
int rfind_space(const String& str) {
    for (int i = static_cast<int>(str.length()) - 1; i >= 0; --i) {
        if (str[i] == ' ') return i;
    }
    return -1;
}

// Remove the first `count` characters from `str`
String remove_leading(const String& str, int count) {
    String result = "";
    for (int i = count; i < (int)str.length(); ++i) {
        result += str[i];
    }
    return result;
}

std::vector<String> confine_text(const String& text, int text_size, iVector2 size = {240, 8}, bool word_wrap = true) {
    if (get_text_width(text, text_size) > size.x) {
        std::vector<String> new_text;
        String remaining_text = text;
        int max_letters = size.x / 6;
        int max_rows = size.y / 8;
        int cur_row = 0;

        if (max_rows <= 1) {
            String new_row = substring(remaining_text, 0, std::max(0, max_letters - 3));
            new_row += "...";
            return {new_row};
        } else {
            while (remaining_text.length() > 0 && cur_row < max_rows) {
                String new_row;

                if ((int)remaining_text.length() <= max_letters) {
                    new_row = remaining_text;
                    new_text.push_back(new_row);
                    return new_text;
                } else {
                    new_row = substring(remaining_text, 0, max_letters);
                }

                if (word_wrap) {
                    int index = rfind_space(new_row);
                    if (index != -1) {
                        new_row = substring(new_row, 0, index + 1);
                    }
                }

                new_text.push_back(new_row);
                remaining_text = remove_leading(remaining_text, new_row.length());
                cur_row++;
            }

            if (remaining_text.length() > 0 && !new_text.empty()) {
                String& last_row = new_text.back();
                if (last_row.length() > 3)
                    last_row = substring(last_row, 0, last_row.length() - 3) + "...";
                else
                    last_row += "...";
            }

            return new_text;
        }
    } else {
        return {text};
    }
}

Box::Box(iVector2 pos, iVector2 size, uint16_t color) {
  this->pos=pos;
  this->size=size;
  this->color=color;
}

void Box::draw(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color) {
  draw_rect(tft, pos, size, color);
}

Button::Button(iVector2 pos, iVector2 size, uint16_t color, String text, int text_size, iVector2 text_buffer, uint16_t text_color, std::function<void(int)> func, int arg = 100) {
  this->pos = pos;
  this->size = size;
  this->color = color;
  this->text = text;
  this->text_size = text_size;
  this->text_buffer = text_buffer;
  this->text_color = text_color;
  this->func = func;
  this->arg = arg;
}

void Button::update(iVector2 point, iVector2 pos, iVector2 size, std::function<void(int)> func, int arg) {
  bool collision = AABB_Point_Collision(iVector2_to_Vector2(point), iVector2_to_Vector2(pos), iVector2_to_Vector2(size));
  if (collision == true) {
    func(arg);
  }
}

void Button::draw(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color, String text, int text_size, uint16_t text_color, iVector2 text_buffer) {
  draw_rect(tft, pos, size, color);
  const char *c = text.c_str();
  draw_centered_text(tft, iVector2{pos.x+text_buffer.x,pos.y+text_buffer.y}, text_color, text_size, c);
}

Songbox::Songbox(iVector2 pos, iVector2 size, uint16_t background_color, iVector2 hit_buffer, iVector2 hit_size, uint16_t hit_color, String song_path, String song_name, int song_index, int text_size, iVector2 text_buffer, uint16_t text_color) {
  this->pos = pos;
  this->size = size;
  this->background_color = background_color;
  this->hit_buffer = hit_buffer;
  this->hit_size = hit_size;
  this->hit_color = hit_color;
  this->song_path = song_path;
  this->song_name = song_name;
  this->song_index = song_index;
  this->text_size = text_size;
  this->text_buffer = text_buffer;
  this->text_color = text_color;
}

void Songbox::draw(int song_selected, Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t background_color, iVector2 hit_buffer, iVector2 hit_size, uint16_t hit_color, String song_name, int song_index, int text_size, iVector2 text_buffer, uint16_t text_color) {
  draw_rect(tft, pos, size, background_color);
  if (song_selected != song_index) {
    draw_rect(tft, iVector2{pos.x+hit_buffer.x, pos.y+hit_buffer.y}, hit_size, hit_color);
  } else {
    draw_rect(tft, iVector2{pos.x+hit_buffer.x, pos.y+hit_buffer.y}, hit_size, tft.color565(101,163,191));
  }
  std::vector<String> new_text = confine_text(song_name, text_size, iVector2{174, 22});
  for (size_t i = 0; i < new_text.size(); ++i) {
    const char *c = new_text.at(i).c_str();
    int y_pos = (pos.y+text_buffer.y) + (i*(8*text_size)) + (i*1);
    draw_text(tft, iVector2{pos.x+text_buffer.x,y_pos}, text_color, text_size, c);
  }
}

void Songbox::draw_hit(int song_selected, Adafruit_ILI9341 tft, iVector2 pos, iVector2 hit_buffer, iVector2 hit_size, uint16_t hit_color, int song_index) {
  if (song_selected != song_index) {
    draw_rect(tft, iVector2{pos.x+hit_buffer.x, pos.y+hit_buffer.y}, hit_size, hit_color);
  } else {
    draw_rect(tft, iVector2{pos.x+hit_buffer.x, pos.y+hit_buffer.y}, hit_size, tft.color565(101,163,191));
  }
}

int Songbox::update(iVector2 point, iVector2 pos, iVector2 size, int song_index) {
  bool collision = AABB_Point_Collision(iVector2_to_Vector2(point), iVector2_to_Vector2(pos), iVector2_to_Vector2(size));
  if (collision == true) {
    return song_index;
  }
  return -1;
}

Page::Page(int start_index, int end_index) {
  this->start_index = start_index;
  this->end_index = end_index;
}

VSlider::VSlider(iVector2 pos, iVector2 size, uint16_t color, uint16_t not_color, iVector2 selector_pos, iVector2 selector_size, uint16_t selector_color, float max_value, float min_value, float value, int buffer) {
  this->pos = pos;
  this->size = size;
  this->color = color;
  this->not_color = not_color;
  //this->selector_pos = selector_pos;
  this->selector_pos = iVector2(selector_pos.x, (int)lerpf((float)(pos.y+size.y-buffer), (float)(pos.y+buffer), value/max_value));
  this->selector_size = selector_size;
  this->selector_color = selector_color;
  this->max_value = max_value;
  this->min_value = min_value;
  this->value = value;
  this->buffer = buffer;
}

void VSlider::draw(Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color, uint16_t not_color, iVector2 selector_pos, iVector2 selector_size, uint16_t selector_color) {
  int half_selector_height = (selector_size.y/2);
  int selector_center = selector_pos.y + half_selector_height;
  int not_height = selector_center-pos.y;
  int height = size.y-not_height;
  draw_rect(tft, pos, iVector2(size.x, not_height-half_selector_height), not_color);
  if (selector_center+half_selector_height < pos.y+size.y) {
    draw_rect(tft, iVector2(pos.x, selector_center+half_selector_height), iVector2(size.x, height-half_selector_height), color);
  }
  draw_rect(tft, selector_pos, selector_size, selector_color);
}

float VSlider::update(iVector2 point, Adafruit_ILI9341 tft, iVector2 pos, iVector2 size, uint16_t color, uint16_t not_color, iVector2 selector_pos, iVector2 selector_size, uint16_t selector_color, float max_value, float min_value, float value, int buffer, uint16_t background_color) {
  bool collision = AABB_Point_Collision(iVector2_to_Vector2(point), iVector2_to_Vector2(iVector2{pos.x, pos.y-9}), iVector2_to_Vector2(iVector2{size.x,size.y+18}));
  int selector_center;
  if (collision == true) {
    tft.drawFastVLine(pos.x-1, pos.y, size.y, background_color);
    tft.drawFastVLine(pos.x+size.x, pos.y, size.y, background_color);
    draw_rect(tft, iVector2{pos.x-1, pos.y-9}, iVector2{size.x+2, 9}, background_color);
    draw_rect(tft, iVector2{pos.x-1, pos.y+size.y}, iVector2{size.x+2, 9}, background_color);
    this->selector_pos.y = point.y-(selector_size.y/2);
    selector_center = this->selector_pos.y + (selector_size.y/2);
    if (selector_center < pos.y) {
      this->selector_pos.y = pos.y - (int)(selector_size.y/2);
    } else if (selector_center > pos.y+size.y) {
      this->selector_pos.y = pos.y+size.y - (int)(selector_size.y/2);
    }
    
    selector_center = selector_pos.y + (selector_size.y/2);
    this->value = max_value-lerpf(min_value, max_value, (float)(selector_center-pos.y+buffer)/(float)(size.y-buffer));
    if (this->value < 0.0) {
      this->value *= -1;
    }
    if (selector_center < pos.y + buffer) {
      this->value = max_value;
    }
    if (selector_center > pos.y+size.y - buffer) {
      this->value = min_value;
    }
    draw(tft, pos, size, color, not_color, selector_pos, selector_size, selector_color);
  }
  return this->value;
}