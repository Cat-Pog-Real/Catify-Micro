void listSongNames() {
  const char* cachePath = "/song_cache/cache.json";

  // Try to load cache
  File cacheFile = SD.open(cachePath, FILE_READ);
  if (cacheFile) {
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, cacheFile);
    cacheFile.close();

    if (!error) {
      JsonArray arr = doc["songs"];
      for (JsonVariant v : arr) {
        songNames.push_back(String(v.as<const char*>()));
      }
      Serial.println("Loaded songs from cache.");
      return;
    } else {
      Serial.println("Cache JSON parse error. Regenerating...");
    }
  }

  // Scan directory and generate list
  File songsDir = SD.open("/songs");
  if (!songsDir || !songsDir.isDirectory()) {
    Serial.println("'/songs' folder not found or not a directory.");
    return;
  }

  StaticJsonDocument<4096> doc;
  JsonArray arr = doc.createNestedArray("songs");

  while (true) {
    File entry = songsDir.openNextFile();
    if (!entry) break;

    if (entry.isDirectory()) {
      String fullPath = String(entry.name());
      songNames.push_back(fullPath);
      arr.add(fullPath);
    }

    entry.close();
  }
  songsDir.close();

  // Save to cache
  File outFile = SD.open(cachePath, FILE_WRITE);
  if (outFile) {
    serializeJson(doc, outFile);
    outFile.close();
    Serial.println("Saved song list to cache.");
  } else {
    Serial.println("Failed to save cache.");
  }
}

String get_mp3_path(const std::vector<String>& songNames, int index) {
  const String basePath = "/songs/" + songNames[index] +"/"+ songNames[index] + ".mp3";
  //Serial.println(basePath);
  return basePath;
}

String get_raw_path(const std::vector<String>& songNames, int index) {
  const String basePath = "/songs/" + songNames[index] +"/"+ songNames[index] + ".raw";
  //Serial.println(basePath);
  return basePath;
}