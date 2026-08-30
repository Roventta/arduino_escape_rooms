#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 32

const uint8_t PIN_COUNT = 8;
const unsigned long FRAME_TIME = 1000 / 45;

const bool correctA[PIN_COUNT] = {
    0,0,1,1,0,0,1,1
};

const bool correctB[PIN_COUNT] = {
    0,1,1,0,0,1,1,0
};

class PinState {
public:
    bool pinA[PIN_COUNT];
    bool pinB[PIN_COUNT];
    uint8_t focusIndex;
    bool submitted_and_correct;

    PinState() {
        for (uint8_t i = 0; i < PIN_COUNT; i++) {
            pinA[i] = false;
            pinB[i] = false;
        }

        focusIndex = 0;
        submitted_and_correct = false;
    }

    void setFocus(uint8_t index) {
        if (index < PIN_COUNT) {
            focusIndex = index;
        }
    }

    void IncreaseFocus() {
        focusIndex = (focusIndex + 1) % PIN_COUNT;
    }

    void SetA() {
        pinA[focusIndex] = !pinA[focusIndex];
    }

    void SetB() {
        pinB[focusIndex] = !pinB[focusIndex];
    }

    void checkAnswer() {
        submitted_and_correct = true;

        for (uint8_t i = 0; i < PIN_COUNT; i++) {
            if (pinA[i] != correctA[i] || pinB[i] != correctB[i]) {
                submitted_and_correct = false;
                return;
            }
        }
    }
};

class OLED {
private:
    Adafruit_SSD1306 display;

    static const uint8_t LABEL_X = 0;
    static const uint8_t DATA_X = 42;
    static const uint8_t PIN_A_Y = 8;
    static const uint8_t PIN_B_Y = 20;
    static const uint8_t FOCUS_SIZE = 4;
    static const uint8_t FOCUS_OFFSET = 2;
    static const uint8_t DATA_SPACING = (OLED_WIDTH - DATA_X) / PIN_COUNT;

    unsigned long both_press_time = 0;
    unsigned long lastFrameTime = 0;
    uint8_t tunnelSize = 0;
    unsigned long lastTunnelFrame = 0;

public:
    OLED() : display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {}

    void begin() {
        Wire.begin(1, 2);

        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            while (true) {}
        }

        display.clearDisplay();
        display.display();
    }

    bool canRender() {
        if (millis() - lastFrameTime < FRAME_TIME) {
            return false;
        }

        lastFrameTime = millis();
        return true;
    }

    void renderBothPressTime() {
        float progress = (float)(both_press_time - BOTH_START_SUBMIT) /
                         (BOTH_FINISH_SUBMIT - BOTH_START_SUBMIT);
        progress = constrain(progress, 0.0f, 1.0f);

        uint8_t width = progress * OLED_WIDTH;
        display.fillRect(0, 0, width, OLED_HEIGHT, SSD1306_WHITE);
    }

    void setBothPressTime(unsigned long in) {
        both_press_time = in;
    }

    void renderTunnel() {
        if (millis() - lastTunnelFrame < FRAME_TIME) {
            return;
        }

        lastTunnelFrame = millis();

        display.clearDisplay();

        const uint8_t centerX = OLED_WIDTH / 2;
        const uint8_t centerY = OLED_HEIGHT / 2;

        for (uint8_t i = 0; i < 4; i++) {
            uint8_t offset = tunnelSize + i * 10;

            if (offset < OLED_WIDTH / 2) {
                display.drawRect(
                    centerX - offset,
                    centerY - offset / 2,
                    offset * 2,
                    offset,
                    SSD1306_WHITE
                );
            }
        }

        tunnelSize++;

        if (tunnelSize >= OLED_WIDTH / 2) {
            tunnelSize = 0;
        }

        display.display();
    }

    void renderPinState(PinState &state) {
        if (state.submitted_and_correct) {
            renderTunnel();
            return;
        }

        if (!canRender()) {
            return;
        }

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        const uint8_t focusX = DATA_X + state.focusIndex * DATA_SPACING;

        display.fillRect(
            focusX + FOCUS_OFFSET,
            0,
            FOCUS_SIZE,
            FOCUS_SIZE,
            SSD1306_WHITE
        );

        display.setCursor(LABEL_X, PIN_A_Y);
        display.print("pin A:");

        for (uint8_t i = 0; i < PIN_COUNT; i++) {
            display.setCursor(DATA_X + i * DATA_SPACING, PIN_A_Y);
            display.print(state.pinA[i]);
        }

        display.setCursor(LABEL_X, PIN_B_Y);
        display.print("pin B:");

        for (uint8_t i = 0; i < PIN_COUNT; i++) {
            display.setCursor(DATA_X + i * DATA_SPACING, PIN_B_Y);
            display.print(state.pinB[i]);
        }

        display.fillRect(
            focusX + FOCUS_OFFSET,
            OLED_HEIGHT - FOCUS_SIZE,
            FOCUS_SIZE,
            FOCUS_SIZE,
            SSD1306_WHITE
        );

        renderBothPressTime();
        display.display();
    }
};

OLED oled;
PinState pins;

void oled_setup() {
    oled.begin();
    pins.setFocus(0);
    oled.renderPinState(pins);
}

#endif