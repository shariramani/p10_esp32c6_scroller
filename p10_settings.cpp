
#include "p10_settings.h"
#include "p10_display.h"

// Default scroll contents
const std::vector<ScrollContent> DEFAULT_SCROLL_CONTENTS = {
  {CONTENT_TIME, "Current Time", true},
  {CONTENT_DATE, "Current Date", true},
  {CONTENT_RSS_FEEDS, "RSS Headlines", true},
  {CONTENT_QUOTE_OF_DAY, "Quote of the Day", false},
  {CONTENT_FUN_FACTS, "Fun Facts", false}
};

void loadDisplaySettings() {
  File file = SPIFFS.open("/display.json", "r");
  if (!file) {
    Serial.println("Display settings file not found - using defaults");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Failed to parse display settings");
    return;
  }
  
  displaySettings.brightness = doc["brightness"] | 8;
  displaySettings.scrollSpeed = doc["scrollSpeed"] | 80;
  displaySettings.scrollDirection = doc["scrollDirection"] | 0;
  displaySettings.isColor = doc["isColor"] | false;
  displaySettings.textColor = doc["textColor"] | 0x1F;
  displaySettings.backgroundColor = doc["backgroundColor"] | 0;
  displaySettings.scrollEnabled = doc["scrollEnabled"] | true;
  
  if (doc.containsKey("scrollContents")) {
    scrollContents.clear();
    JsonArray contents = doc["scrollContents"].as<JsonArray>();
    
    for (JsonObject content : contents) {
      ContentType type = static_cast<ContentType>(content["type"].as<int>());
      String name = content["name"].as<String>();
      bool enabled = content["enabled"].as<bool>();
      String contentText = content["content"].as<String>();
      
      ScrollContent sc(type, name, enabled);
      sc.content = contentText;
      scrollContents.push_back(sc);
    }
  }
  
  Serial.println("Display settings loaded successfully");
}

void saveDisplaySettings() {
  File file = SPIFFS.open("/display.json", "w");
  if (!file) {
    Serial.println("Failed to open display settings for writing");
    return;
  }
  
  DynamicJsonDocument doc(2048);
  doc["brightness"] = displaySettings.brightness;
  doc["scrollSpeed"] = displaySettings.scrollSpeed;
  doc["scrollDirection"] = displaySettings.scrollDirection;
  doc["isColor"] = displaySettings.isColor;
  doc["textColor"] = displaySettings.textColor;
  doc["backgroundColor"] = displaySettings.backgroundColor;
  doc["scrollEnabled"] = displaySettings.scrollEnabled;
  
  JsonArray contents = doc.createNestedArray("scrollContents");
  for (const auto& content : scrollContents) {
    JsonObject obj = contents.createNestedObject();
    obj["type"] = static_cast<int>(content.type);
    obj["name"] = content.name;
    obj["enabled"] = content.enabled;
    obj["content"] = content.content;
  }
  
  size_t bytesWritten = serializeJson(doc, file);
  file.close();
  
  Serial.printf("Display settings saved (%d bytes)\n", bytesWritten);
}

void initializeDefaultScrollContents() {
  if (scrollContents.empty()) {
    scrollContents = DEFAULT_SCROLL_CONTENTS;
    Serial.println("Loaded default scroll contents");
  }
}
