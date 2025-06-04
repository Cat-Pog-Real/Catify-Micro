const int SIZE = 5;

void play_song(const char c[]) {
  if (isPlaying == false) {
    isPlaying = true;
    song_time_elapsed = -1;
    audio.connecttoFS(SD, c);
  }
}

void pause_song() {
  if (isPlaying == true) {
    isPlaying = false;
    audio.pauseResume();
  }
}

void toggle_song() {
  audio.pauseResume();
  Serial.println(audio.isRunning());
  if (audio.isRunning() == true) {
    isPlaying = true;
  } else {
    isPlaying = false;
  }
}

void init_shuffle() {
  shuffleSongIndexes.clear();
  for (size_t i = 0; i < songbox_dict.size(); ++i) {
    Songbox songbox = songbox_dict.at(i);
    shuffleSongIndexes.push_back(songbox.song_index);
  }
}

int pick_random_index() {
  if (shuffleSongIndexes.empty() == true) {
    init_shuffle();
  }
  srand(time(0));
  int randomIndex = rand() % shuffleSongIndexes.size();
  int randomSongIndex = shuffleSongIndexes.at(randomIndex);
  shuffleSongIndexes.erase(shuffleSongIndexes.begin() + randomIndex);
  return randomSongIndex;
}

void pick_random_song() {
  int rand = pick_random_index();
  song_selected = rand;
  isPlaying = false;
  Songbox songbox = songbox_dict.at(song_selected);
  const char *c = songbox.song_path.c_str();
  play_song(c);
}

void shuffle_song(int dir) {
  song_selected_pos += dir;
  if (dir > 0) {
    if (song_selected_pos > (SIZE-1)) {
      for (size_t i = 0; i < SIZE - 1; ++i) {
        song_selected_list[i] = song_selected_list[i + 1];
      }
      song_selected_list[SIZE - 1] = -1;
      song_selected_pos = SIZE - 1;
    }
  } else {
    if (song_selected_pos < 0) {
      for (int i = SIZE - 1; i > 0; --i) {
        song_selected_list[i] = song_selected_list[i - 1];
      }
      song_selected_list[0] = -1;
      song_selected_pos = 0;
    }
  }
  if (shuffle_songs == true) {
    if (song_selected_list[song_selected_pos] == -1) {
      int randomIndex = pick_random_index();
      song_selected = randomIndex;
      song_selected_list[song_selected_pos] = song_selected;
    } else {
      song_selected = song_selected_list[song_selected_pos];
    }
  } else {
    if (song_selected_list[song_selected_pos] == -1) {
      song_selected += dir;
      int max_index = songbox_dict.size()-1;
      if (song_selected > max_index) {
        song_selected = 0;
      }
      if (song_selected < 0) {
        song_selected = max_index;
      }
      song_selected_list[song_selected_pos] = song_selected;
    } else {
      song_selected = song_selected_list[song_selected_pos];
    }
  }
  Songbox songbox = songbox_dict.at(song_selected);
  const char *c = songbox.song_path.c_str();
  isPlaying = false;
  play_song(c);
}

void skip_foward() {
  shuffle_song(1);
}

void skip_backward() {
  shuffle_song(-1);
}