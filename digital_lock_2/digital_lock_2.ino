const unsigned long DEBOUNCE_DELAY = 50;

class Button {
public:
    int pinId;

    bool lastFrameButtonState;
    bool stableButtonState;
    bool lastStableButtonState;

    unsigned long lastDebounceTime;
    unsigned long lastTimePressed;
    unsigned long lastTimeReleased;

    void setup(int pin) {
        pinId = pin;
        pinMode(pinId, INPUT_PULLUP);

        // Read the actual initial state
        stableButtonState = digitalRead(pinId);
        lastStableButtonState = stableButtonState;
        lastFrameButtonState = stableButtonState;

        lastDebounceTime = millis();

        lastTimePressed = 0;
        lastTimeReleased = 0;
    }

    bool debounce() {
        bool currentState = digitalRead(pinId);

        // Input changed this frame
        if (currentState != lastFrameButtonState) {
            lastDebounceTime = millis();
        }
        lastFrameButtonState = currentState;
        // Has the input remained unchanged for long enough?
        if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
            // Stable state has changed
            if (currentState != stableButtonState) {
                lastStableButtonState = stableButtonState;
                stableButtonState = currentState;
                return true;
            }
        }
        return false;
    }

    void getPressTime() {
        Serial.println(lastTimeReleased - lastTimePressed);
    }

    void update() {
        if (!debounce()) {
            return;
        }

        Serial.println("touched");

        // Pressed
        if (lastStableButtonState == HIGH &&
            stableButtonState == LOW) {
            lastTimePressed = millis();
        }

        // Released
        if (lastStableButtonState == LOW &&
            stableButtonState == HIGH) {
            lastTimeReleased = millis();
        }
    }
};

Button button_1;
Button button_2;

const unsigned long BOTH_INPUT = 50; 
unsigned long both_start_time = 0;
bool is_both = false; 

void update_checkBoth(){
    bool check_both = button_1.stableButtonState==LOW && button_2.stableButtonState ==LOW;
    unsigned long now = millis();
    if(!is_both && check_both){
        is_both = true;
        both_start_time = now;
    }
    if(is_both && check_both){
        is_both = true;
        unsigned long both_last_time = now - both_start_time;
        if(both_last_time > BOTH_INPUT){
            Serial.print("Bothed, resetting");
            is_both=false;
        }
    }
    if(!check_both){
        is_both = false;
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Begin");
    button_1.setup(13);
    button_2.setup(21);
}

void loop() {
    button_1.update();
    button_2.update();

    update_checkBoth();
}