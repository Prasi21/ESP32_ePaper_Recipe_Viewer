
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <string.h>
#include "epd7in5_V2.h"

// One bit per pixel: 800 * 480 / 8 = 48,000 bytes.
static const size_t IMAGE_BYTES =
    (EPD_WIDTH * EPD_HEIGHT) / 8;

static uint8_t imageBuffer[IMAGE_BYTES];
static Epd epd;
static bool displayDriverReady = false;

// Initialise the Waveshare V2 driver.
bool initDisplay() {
    Serial.println("Initialising Waveshare 7.5 V2");

    if (epd.Init() != 0) {
        Serial.println("Display initialisation failed");
        displayDriverReady = false;
        return false;
    }

    displayDriverReady = true;
    Serial.println("Display initialised");
    return true;
}

// The driver handles BUSY waiting during initialisation and refresh.
void waitForIdle() {
    if (displayDriverReady) {
        epd.WaitUntilIdle();
    }
}

bool isDisplayBusy() {
    if (!displayDriverReady) {
        return true;
    }

    return digitalRead(BUSY_PIN) == LOW;
}

// Prepare to receive a new image.
void displayStartTransmission() {
    memset(imageBuffer, 0, sizeof(imageBuffer));
}

// Copy a WebSocket data chunk into its correct position.
void loadImage(const char* data, size_t length, size_t offset) {
    if (offset >= IMAGE_BYTES ||
        length > IMAGE_BYTES - offset) {
        Serial.println("Image data exceeds framebuffer size");
        return;
    }

    memcpy(imageBuffer + offset, data, length);
}


// Display the complete received image.
// DisplayFrame handles transmission, refresh and BUSY waiting.
void updateDisplay_withoutIdle() {
    if (!displayDriverReady) {
        Serial.println("Display driver is not ready");
        return;
    }

    Serial.println("Refreshing e-paper display");
    epd.DisplayFrame(imageBuffer);
    Serial.println("Display refresh complete");
}



#endif
