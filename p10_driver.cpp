
#include "p10_driver.h"
#include "p10_display.h"

// Hardware timer and scanning variables
hw_timer_t* displayTimer = NULL;
volatile uint8_t currentScanRow = 0;
uint8_t displayBuffer[DISPLAY_HEIGHT][DISPLAY_WIDTH];

void IRAM_ATTR displayISR() {
  scanNextRow();
}

void initializeP10Hardware() {
  Serial.println("Initializing P10 LED Display (Mono Red, 32x16, 4-row scan)...");
  
  // Initialize GPIO pins
  pinMode(P10_LAT, OUTPUT);
  pinMode(P10_OE, OUTPUT);
  pinMode(P10_A, OUTPUT);
  pinMode(P10_B, OUTPUT);
  pinMode(P10_CLK, OUTPUT);
  pinMode(P10_R1, OUTPUT);
  
  // Set initial pin states
  digitalWrite(P10_LAT, LOW);
  digitalWrite(P10_OE, HIGH);  // Disable output initially
  digitalWrite(P10_A, LOW);
  digitalWrite(P10_B, LOW);
  digitalWrite(P10_CLK, LOW);
  digitalWrite(P10_R1, LOW);
  
  // Clear display buffer
  clearDisplayBuffer();
  
  // Setup timer for display refresh - optimized for mono red
  displayTimer = timerBegin(1000000); // 1MHz
  timerAttachInterrupt(displayTimer, &displayISR);
  timerAlarm(displayTimer, 5000, true, 0); // 5000us = 200Hz refresh rate (slower for stability)
  
  Serial.printf("P10 Hardware initialized - Panel: 32x16 mono red\n");
}

void scanNextRow() {
  static uint8_t brightnessCounter = 0;
  
  // Disable output during data shift
  digitalWrite(P10_OE, HIGH);
  
  // Set row address for 4-row scanning (A and B pins only)
  digitalWrite(P10_A, (currentScanRow & 0x01) ? HIGH : LOW);
  digitalWrite(P10_B, (currentScanRow & 0x02) ? HIGH : LOW);
  
  // Shift out data for current row and corresponding bottom row
  for (int col = DISPLAY_WIDTH - 1; col >= 0; col--) {
    digitalWrite(P10_CLK, LOW);
    
    // Combine upper and lower half rows for mono red panel
    bool pixelOn = displayBuffer[currentScanRow][col] || 
                   displayBuffer[currentScanRow + DISPLAY_SCAN_ROWS][col];
    
    digitalWrite(P10_R1, pixelOn ? HIGH : LOW);
    
    // Clock pulse
    delayMicroseconds(1);
    digitalWrite(P10_CLK, HIGH);
    delayMicroseconds(1);
  }
  
  // Latch the data
  digitalWrite(P10_CLK, LOW);
  digitalWrite(P10_LAT, HIGH);
  delayMicroseconds(2);
  digitalWrite(P10_LAT, LOW);
  
  // PWM brightness control
  brightnessCounter++;
  if (brightnessCounter <= displaySettings.brightness) {
    digitalWrite(P10_OE, LOW);  // Enable output
  } else {
    digitalWrite(P10_OE, HIGH); // Disable output
  }
  
  if (brightnessCounter >= 15) {
    brightnessCounter = 0;
  }
  
  // Move to next scan row
  currentScanRow = (currentScanRow + 1) % DISPLAY_SCAN_ROWS;
}

void clearDisplayBuffer() {
  for (int row = 0; row < DISPLAY_HEIGHT; row++) {
    for (int col = 0; col < DISPLAY_WIDTH; col++) {
      displayBuffer[row][col] = 0;
    }
  }
}

void setDisplayBrightness(uint8_t brightness) {
  displaySettings.brightness = constrain(brightness, 1, 15);
  Serial.printf("Display brightness set to: %d\n", displaySettings.brightness);
}
