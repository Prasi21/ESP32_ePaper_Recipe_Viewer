
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
// void loadImage(const char* data, size_t length, size_t offset) {
//     if (offset >= IMAGE_BYTES ||
//         length > IMAGE_BYTES - offset) {
//         Serial.println("Image data exceeds framebuffer size");
//         return;
//     }

//     memcpy(imageBuffer + offset, data, length);
// }
void loadImage(const char* data, size_t length, size_t offset) {
    if (offset + length > IMAGE_BYTES) {
        Serial.printf(
            "Image overflow: offset=%u length=%u capacity=%u\n",
            (unsigned int)offset,
            (unsigned int)length,
            (unsigned int)IMAGE_BYTES
        );
        return;
    }

    memcpy(&imageBuffer[offset], data, length);
}


// Temp helper function
void printFramebufferSamples() {
    Serial.println("Framebuffer samples:");

    for (int y = 0; y < 480; y += 40) {
        Serial.printf(
            "Row %d: %02X %02X %02X %02X %02X\n",
            y,
            imageBuffer[y * 100],
            imageBuffer[y * 100 + 1],
            imageBuffer[y * 100 + 2],
            imageBuffer[y * 100 + 50],
            imageBuffer[y * 100 + 99]
        );
    }
}

// Display the complete received image.
// DisplayFrame handles transmission, refresh and BUSY waiting.
void updateDisplay_withoutIdle() {
    if (!displayDriverReady) {
        Serial.println("Display driver is not ready");
        return;
    }

    Serial.println("Refreshing e-paper display");
    printFramebufferSamples();
    epd.DisplayFrame(imageBuffer);
    Serial.println("Display refresh complete");
}



#endif
