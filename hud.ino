int page_index = 0;

void load_Songbox() {
  int y_pos = 37;
  int start_index = -1;
  int end_index = 0;
  int counter = 0;
  for (size_t i = 0; i < songNames.size(); ++i) {
    counter += 1;
    if (start_index == -1) {
      start_index = i;
      y_pos = 37;
    }
    Songbox songbox(iVector2{2, y_pos}, iVector2{206,30}, color.grey, iVector2{178,2}, iVector2{26,26}, color.background, get_mp3_path(songNames, i), songNames[i], i, 1, iVector2{4, 7}, color.white);
    songbox_dict.insert({i, songbox});
    y_pos += 34;
    if (counter >= 8) {
      end_index = i;
      Page p = Page(start_index, end_index);
      page_dict.insert({page_index, p});
      page_index += 1;
      start_index = -1;
      counter = 0;
    }
  }
  if (counter != 0) {
    end_index = start_index + (counter-1);
    Page p = Page(start_index, end_index);
    page_dict.insert({page_index, p});
    page_index += 1;
  }
}

void draw_songbox(int index) {
  songbox_dict.at(index).draw(song_selected, tft, songbox_dict.at(index).pos, songbox_dict.at(index).size, songbox_dict.at(index).background_color, songbox_dict.at(index).hit_buffer, songbox_dict.at(index).hit_size, songbox_dict.at(index).hit_color, songbox_dict.at(index).song_name, songbox_dict.at(index).song_index, songbox_dict.at(index).text_size, songbox_dict.at(index).text_buffer, songbox_dict.at(index).text_color);
}

void load_page(int index) {
    Page page = page_dict.at(index);
    int counter = 0;
    int width = songbox_dict.at(0).size.x;
    int x = songbox_dict.at(0).pos.x;
    int length = (page.end_index - page.start_index)+1;
    for (size_t i = 0; i < length; ++i) {
      counter += 1;
      draw_songbox(page.start_index + i);
    }
    if (counter != 8) {
      int last_pos = songbox_dict.at(page.end_index).pos.y+songbox_dict.at(page.end_index).size.y;
      draw_rect(tft, iVector2{x, last_pos+1}, iVector2{width, 308-(last_pos+1)}, color.background);
    }
}

void draw_raw(Adafruit_ILI9341 &tft, String img_path, iVector2 pos) {
  File imgFile = SD.open(img_path);
  if (!imgFile) {
    Serial.println("Image not found!");
    return;
  }

  const int IMG_W = 180;
  const int IMG_H = 180;
  const int BATCH_H = 6;  // Number of lines per batch
  uint16_t lineBuffer[IMG_W * BATCH_H];  // 180 * 6 * 2 bytes = 2.1 KB RAM

  for (int y = 0; y < IMG_H; y += BATCH_H) {
    int linesThisBatch = min(BATCH_H, IMG_H - y);

    // Read BATCH_H lines into buffer
    for (int i = 0; i < IMG_W * linesThisBatch; i++) {
      int hi = imgFile.read();
      int lo = imgFile.read();
      if (hi == -1 || lo == -1) {
        Serial.println("Unexpected end of file!");
        imgFile.close();
        return;
      }
      lineBuffer[i] = (hi << 8) | lo;
    }

    // Draw the batch
    tft.startWrite();
    tft.setAddrWindow(pos.x, pos.y + y, IMG_W, linesThisBatch);
    tft.writePixels(lineBuffer, IMG_W * linesThisBatch, true);
    tft.endWrite();
  }

  imgFile.close();
}

void update_page(int index, iVector2 pos) {
  Page page = page_dict.at(index);
  bool update = false;
  int old_selected = song_selected;
  int length = (page.end_index - page.start_index)+1;
  for (size_t i = 0; i < length; ++i) {
    Songbox songbox = songbox_dict.at(page.start_index + i);
    int result = songbox.update(iVector2{pos.x, pos.y}, iVector2{songbox.pos.x+songbox.hit_buffer.x, songbox.pos.y+songbox.hit_buffer.y}, songbox.hit_size, songbox.song_index);
    if (result != -1) {
      update = true;
      song_selected = result;
      const char *c = songbox.song_path.c_str();
      song_selected_list[song_selected_pos] = song_selected;
      isPlaying = false;
      play_song(c);
    }
  }
  if (update == true && old_selected != song_selected) {
    if (old_selected != -1) {
      Songbox old_songbox = songbox_dict.at(old_selected);
      old_songbox.draw_hit(song_selected, tft, old_songbox.pos, old_songbox.hit_buffer, old_songbox.hit_size, old_songbox.hit_color, old_songbox.song_index);
    }
    if (song_selected != -1) {
      Songbox new_songbox = songbox_dict.at(song_selected);
      new_songbox.draw_hit(song_selected, tft, new_songbox.pos, new_songbox.hit_buffer, new_songbox.hit_size, new_songbox.hit_color, new_songbox.song_index);
    }
  }
  String old_name = song_selected_name;
  if (song_selected != -1) {
    Songbox last_songbox = songbox_dict.at(song_selected);
    song_selected_name = last_songbox.song_name;
    if (old_name != song_selected_name) {
      bottom_panel.draw(tft, bottom_panel.pos, bottom_panel.size, bottom_panel.color);
      std::vector<String> new_text = confine_text(song_selected_name, 1, iVector2{200, 8}, false);
      const char *c = new_text.at(0).c_str();
      draw_text(tft, iVector2{2,310}, color.white, 1, c);
    }
  }
}