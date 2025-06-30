
#include "p10_content.h"
#include "p10_display.h"
#include "p10_renderer.h"

void updateDisplayContent() {
  static unsigned long lastContentUpdate = 0;
  static int contentIndex = 0;
  
  // Update content every 5 seconds
  if (millis() - lastContentUpdate > 5000) {
    
    // Find next enabled content
    int attempts = 0;
    do {
      contentIndex = (contentIndex + 1) % scrollContents.size();
      attempts++;
    } while (!scrollContents[contentIndex].enabled && attempts < scrollContents.size());
    
    if (scrollContents[contentIndex].enabled) {
      String newContent = "";
      
      switch (scrollContents[contentIndex].type) {
        case CONTENT_TIME:
          newContent = generateTimeContent();
          break;
        case CONTENT_DATE:
          newContent = generateDateContent();
          break;
        case CONTENT_RSS_FEEDS:
          newContent = generateRSSContent();
          break;
        case CONTENT_QUOTE_OF_DAY:
          newContent = loadQuoteOfDay();
          break;
        case CONTENT_FUN_FACTS:
          newContent = loadFunFact();
          break;
        case CONTENT_CUSTOM_TEXT:
          newContent = scrollContents[contentIndex].content;
          break;
      }
      
      if (newContent != displaySettings.currentContent) {
        displaySettings.currentContent = newContent;
        displaySettings.scrollPosition = 0;
        
        Serial.printf("Display content updated: %s\n", newContent.c_str());
      }
    }
    
    lastContentUpdate = millis();
  }
  
  // Handle scrolling
  if (displaySettings.scrollEnabled) {
    scrollText();
  }
}

void scrollText() {
  if (millis() - displaySettings.lastScrollTime < displaySettings.scrollSpeed) {
    return;
  }
  
  if (displaySettings.currentContent.length() == 0) {
    return;
  }
  
  // Calculate text width properly
  int textWidth = calculateTextWidth(displaySettings.currentContent);
  bool needsScrolling = textWidth > DISPLAY_WIDTH;
  
  Serial.printf("Content: '%s', Width: %d, Display: %d, Needs scrolling: %s\n", 
                displaySettings.currentContent.c_str(), textWidth, DISPLAY_WIDTH, needsScrolling ? "YES" : "NO");
  
  if (needsScrolling) {
    // Scrolling text
    displaySettings.scrollPosition++;
    
    if (displaySettings.scrollPosition >= textWidth + DISPLAY_WIDTH) {
      displaySettings.scrollPosition = 0;
    }
    
    // Clear and redraw scrolling text
    clearDisplayBuffer();
    int drawX = DISPLAY_WIDTH - displaySettings.scrollPosition;
    drawText(displaySettings.currentContent, drawX, 5); // Vertically centered
    
    displaySettings.lastScrollTime = millis();
    
  } else {
    // Static text that fits
    static unsigned long lastStaticUpdate = 0;
    if (millis() - lastStaticUpdate > 1000) { // Update every second for time
      int startX = (DISPLAY_WIDTH - textWidth) / 2;
      if (startX < 0) startX = 0;
      drawText(displaySettings.currentContent, startX, 5); // Vertically centered
      lastStaticUpdate = millis();
      Serial.printf("Static display: '%s' centered at x=%d\n", displaySettings.currentContent.c_str(), startX);
    }
  }
}

void setScrollSpeed(uint8_t speed) {
  displaySettings.scrollSpeed = constrain(speed, 30, 200);
  Serial.printf("Scroll speed set to: %d ms\n", displaySettings.scrollSpeed);
}

void setScrollDirection(uint8_t direction) {
  displaySettings.scrollDirection = constrain(direction, 0, 3);
  displaySettings.scrollPosition = 0;
  Serial.printf("Scroll direction set to: %d\n", displaySettings.scrollDirection);
}

void addScrollContent(ContentType type, const String& content) {
  ScrollContent newContent(type, content, true);
  newContent.content = content;
  scrollContents.push_back(newContent);
  Serial.printf("Added scroll content: %s\n", content.c_str());
}

void addRSSHeadline(const String& headline) {
  allRSSHeadlines.push_back(headline);
  
  if (allRSSHeadlines.size() > 20) {
    allRSSHeadlines.erase(allRSSHeadlines.begin());
  }
  
  Serial.printf("Added RSS headline: %s (Total: %d)\n", headline.c_str(), allRSSHeadlines.size());
}

void clearRSSHeadlines() {
  allRSSHeadlines.clear();
  Serial.println("Cleared all RSS headlines");
}

String generateTimeContent() {
  return getCurrentTimeString();
}

String generateDateContent() {
  return getCurrentDateString();
}

String generateRSSContent() {
  static int headlineIndex = 0;
  
  if (allRSSHeadlines.size() > 0) {
    String headline = allRSSHeadlines[headlineIndex];
    headlineIndex = (headlineIndex + 1) % allRSSHeadlines.size();
    return headline;
  }
  
  return "No RSS headlines available";
}

String loadQuoteOfDay() {
  File file = SPIFFS.open("/quotes.txt", "r");
  if (!file) {
    return "Believe you can and you're halfway there.";
  }
  
  std::vector<String> quotes;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      quotes.push_back(line);
    }
  }
  file.close();
  
  if (quotes.size() > 0) {
    int randomIndex = random(0, quotes.size());
    return quotes[randomIndex];
  }
  
  return "No quotes available";
}

String loadFunFact() {
  File file = SPIFFS.open("/facts.txt", "r");
  if (!file) {
    return "The ESP32 has built-in Wi-Fi and Bluetooth!";
  }
  
  std::vector<String> facts;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      facts.push_back(line);
    }
  }
  file.close();
  
  if (facts.size() > 0) {
    int randomIndex = random(0, facts.size());
    return facts[randomIndex];
  }
  
  return "No facts available";
}
