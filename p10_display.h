
#ifndef P10_DISPLAY_H
#define P10_DISPLAY_H

#include "config.h"

// P10 Display pin definitions for ESP32-C6 (Mono Red Panel)
#define P10_LAT 5
#define P10_OE  6
#define P10_A   2
#define P10_B   3
// No P10_C pin for mono red panels
#define P10_CLK 18
#define P10_R1  19

// Display dimensions for mono red P10 panel
#define DISPLAY_WIDTH 32
#define DISPLAY_HEIGHT 16
#define DISPLAY_SCAN_ROWS 4  // Mono red P10 has 4 scan rows (not 8)

// Font dimensions
#define FONT_WIDTH 5
#define FONT_HEIGHT 7
#define CHAR_SPACING 6  // 5 pixels + 1 space

// P10 Display Configuration
struct DisplaySettings {
  uint8_t brightness = 8;       // 0-15, reduced default for stability
  uint8_t scrollSpeed = 80;     // milliseconds delay, slower for readability
  uint8_t scrollDirection = 0;  // 0=left, 1=right, 2=up, 3=down
  bool isColor = false;         // true=color, false=mono
  uint16_t textColor = 0x1F;    // Red color for mono (5-bit red)
  uint16_t backgroundColor = 0; // Background color
  String currentContent = "";
  unsigned long lastScrollTime = 0;
  int scrollPosition = 0;
  bool scrollEnabled = true;
};

// Content types for scrolling
enum ContentType {
  CONTENT_RSS_FEEDS = 0,
  CONTENT_TIME = 1,
  CONTENT_DATE = 2,
  CONTENT_QUOTE_OF_DAY = 3,
  CONTENT_FUN_FACTS = 4,
  CONTENT_CUSTOM_TEXT = 5
};

struct ScrollContent {
  ContentType type;
  String name;
  bool enabled;
  String content;
  
  ScrollContent(ContentType t, const String& n, bool e = true) 
    : type(t), name(n), enabled(e) {}
};

// Global display variables
extern DisplaySettings displaySettings;
extern std::vector<ScrollContent> scrollContents;
extern std::vector<String> allRSSHeadlines;

// Main P10 display functions
void initializeP10Display();

// Include module headers for their function declarations
#include "p10_driver.h"
#include "p10_renderer.h"
#include "p10_content.h"
#include "p10_settings.h"

#endif
