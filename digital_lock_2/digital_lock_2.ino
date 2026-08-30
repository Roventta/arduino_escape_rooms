#define BOTH_START_SUBMIT 700
#define BOTH_FINISH_SUBMIT 2000

#include "oled.h"

const unsigned long DEBOUNCE_DELAY = 100;
const unsigned long WINDOW_WAIT_BOTH = 200;

int button_inputted = -1;

class Button {
public:
    int pinId;
    Button* b;

    bool lastFrameButtonState;
    bool stableButtonState;
    bool lastStableButtonState;

    unsigned long lastDebounceTime;
    unsigned long lastTimePressed;
    unsigned long lastTimeReleased;

    void set_another_button(Button *b) {
        this->b = b;
    }

    bool check_is_near_another_movement() {
        return (abs(int(this->lastDebounceTime) - int(b->lastDebounceTime)) < WINDOW_WAIT_BOTH);
    }

    void setup(int pin) {
        pinId = pin;
        pinMode(pinId, INPUT_PULLUP);

        stableButtonState = digitalRead(pinId);
        lastStableButtonState = stableButtonState;
        lastFrameButtonState = stableButtonState;

        lastDebounceTime = millis();
        lastTimePressed = 0;
        lastTimeReleased = 0;
    }

    bool debounce() {
        bool currentState = digitalRead(pinId);

        if (currentState != lastFrameButtonState) {
            lastDebounceTime = millis();
        }

        lastFrameButtonState = currentState;

        if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
            if (currentState != stableButtonState) {
                lastStableButtonState = stableButtonState;
                stableButtonState = currentState;
                return true;
            }
        }

        return false;
    }

    unsigned long getPressTime() {
        return lastTimeReleased - lastTimePressed;
    }

    void update() {
        if (!debounce()) {
            return;
        }

        // BUTTON PRESS
        if (lastStableButtonState == HIGH &&
            stableButtonState == LOW) {
            lastTimePressed = millis();
        }

        // PRESSING
        if (stableButtonState == LOW) {
            if (!this->check_is_near_another_movement() &&
                this->getPressTime() > WINDOW_WAIT_BOTH) {

                if (button_inputted == -1) {
                    button_inputted = pinId;
                    Serial.printf("%d pressed\n", this->pinId);
                    if(this->pinId==13){
                        pins.SetA();
                    }
                    else{
                        pins.SetB();
                    }
                }
            }
        }

        // BUTTON RELEASE
        if (lastStableButtonState == LOW &&
            stableButtonState == HIGH) {

            lastTimeReleased = millis();

            if (button_inputted == pinId) {
                button_inputted = -1;
            }
        }
    }
};

Button button_1;
Button button_2;

const unsigned long BOTH_INPUT = 50;

unsigned long both_start_time = 0;
bool is_both = false;

// True after submitting.
// While true, both-button detection is disabled
// until both buttons are HIGH.
bool both_locked = false;

// BOTH BUTTON CHECKER

void reset_both_checker() {
    is_both = false;
    both_start_time = 0;
}

void update_checkBoth() {
    bool check_both =
        button_1.stableButtonState == LOW &&
        button_2.stableButtonState == LOW;

    bool both_high =
        button_1.stableButtonState == HIGH &&
        button_2.stableButtonState == HIGH;

    unsigned long now = millis();

    // LOCKED STATE
    if (both_locked) {
        if (both_high) {
            both_locked = false;
            reset_both_checker();

            Serial.print("both buttons released\n");
        }

        return;
    }

    // BOTH BUTTON GUARD
    if (!is_both && button_inputted != -1) {
        return;
    }

    // BOTH BUTTONS PRESSED
    if (!is_both && check_both) {
        if (button_inputted != -1) {
            return;
        }

        is_both = true;
        both_start_time = now;
    }

    // BOTH BUTTONS CONTINUE TO BE PRESSED
    if (is_both && check_both) {
        unsigned long both_last_time = now - both_start_time;

        if (both_last_time > BOTH_START_SUBMIT &&
            both_last_time < BOTH_FINISH_SUBMIT) {
            Serial.println("submitting");
            oled.setBothPressTime(both_last_time);
        }

        if (both_last_time > BOTH_FINISH_SUBMIT) {
            Serial.print("submitted\n");
            pins.checkAnswer();
            both_locked = true;
            reset_both_checker();
            oled.setBothPressTime(BOTH_START_SUBMIT);
        }
    }

    // ONE OR BOTH BUTTONS RELEASED
    if (is_both && !check_both) {
        unsigned long both_last_time = now - both_start_time;

        if (both_last_time > BOTH_INPUT &&
            both_last_time < BOTH_START_SUBMIT) {
            Serial.print("both inputted\n");
            // increase focus
            pins.IncreaseFocus();
        }
    }

    // NO LONGER BOTH PRESSED
    if (!check_both) {
        reset_both_checker();
        oled.setBothPressTime(BOTH_START_SUBMIT);
    }
}

// SETUP

void setup() {
    Serial.begin(115200);
    Serial.println("Begin");

    button_1.setup(13);
    button_2.setup(21);
    button_1.set_another_button(&button_2);
    button_2.set_another_button(&button_1);

    oled_setup();
}

// LOOP

void loop() {
    button_1.update();
    button_2.update();

    update_checkBoth();

    oled.renderPinState(pins);
}