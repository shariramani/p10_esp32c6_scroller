
#ifndef P10_DRIVER_H
#define P10_DRIVER_H

#include <Arduino.h>
#include "p10_display.h"

// Function declarations for P10 hardware control
void initializeP10Hardware();
void scanNextRow();
void clearDisplayBuffer();
void setDisplayBrightness(uint8_t brightness);

// Hardware variables
extern hw_timer_t* displayTimer;
extern volatile uint8_t currentScanRow;
extern uint8_t displayBuffer[][32]; // Forward declaration with fixed size

#endif