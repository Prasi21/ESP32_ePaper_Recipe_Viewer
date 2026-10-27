
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <string.h>
#include "epd7in5_V2.h"

// One bit per pixel: 800 * 480 / 8 = 48,000 bytes.
static const size_t IMAGE_BYTES =
    (EPD_WIDTH * EPD_HEIGHT) / 8;

static uint8_t imageBuffer[IMAGE_BYTES];
static size_t imageWriteOffset = 0;
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
    imageWriteOffset = 0;
}

// Append a WebSocket data chunk. Frames/chunks arrive in order, so we cannot
// use AwsFrameInfo::index here: it resets to zero for every WebSocket frame.
void loadImage(const char* data, size_t length) {
    if (imageWriteOffset >= IMAGE_BYTES ||
        length > IMAGE_BYTES - imageWriteOffset) {
        Serial.println("Image data exceeds framebuffer size");
        return;
    }

    memcpy(imageBuffer + imageWriteOffset, data, length);
    imageWriteOffset += length;
}


// Display the complete received image.
// DisplayFrame handles transmission, refresh and BUSY waiting.
void updateDisplay_withoutIdle() {
    if (!displayDriverReady) {
        Serial.println("Display driver is not ready");
        return;
    }

    if (imageWriteOffset != IMAGE_BYTES) {
        Serial.printf(
            "Incomplete image: %u / %u bytes, skipping refresh\n",
            (unsigned int)imageWriteOffset,
            (unsigned int)IMAGE_BYTES
        );
        return;
    }

    Serial.println("Refreshing e-paper display");
    epd.DisplayFrame(imageBuffer);
    Serial.println("Display refresh complete");
}



#endif
