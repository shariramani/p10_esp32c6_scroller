
#ifndef P10_RENDERER_H
#define P10_RENDERER_H

#include <Arduino.h>
#include "p10_display.h"

// Font and rendering functions
uint8_t getCharPattern(char c, int row);
int calculateTextWidth(const String& text);
void drawText(const String& text, int x, int y);
void shiftDisplayLeft();

#endif