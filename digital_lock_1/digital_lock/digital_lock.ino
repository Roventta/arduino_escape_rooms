#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define ROTARY_PINA 17
#define ROTARY_PINB 18

#define SCL_1 2
#define SDA_1 1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

 int counter = 0; 
 int aState;
 int aLastState;  
 const int optionCount = 4;
 char options[optionCount] = {'A', 'B', 'C', 'D'};
 char option = 'A';
 int i = 0;
 int counter_gap = 6;

struct OledCursor {
  int width = 6;
  int height = 8;
  int x = 0;
  int y = 10;
  bool on_of = true;
  unsigned long last_blink = 0;
  unsigned long period = 500;
};

OledCursor oled_cursor;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void oled_print(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, world!");
  display.display(); 
} 

void setup() { 
   pinMode (ROTARY_PINA,INPUT);
   pinMode (ROTARY_PINB,INPUT);
   Serial.begin (9600);
   // Reads the initial state of the ROTARY_PINA
   aLastState = digitalRead (ROTARY_PINA);

  Wire.begin(SDA_1, SCL_1);
  oled_print();
 } 

void updateCounter(){
  aState = digitalRead(ROTARY_PINA); // Reads the "current" state of the ROTARY_PINA
   // If the previous and the current state of the ROTARY_PINA are different, that means a Pulse has occured
   int bState = digitalRead(ROTARY_PINB);
   if (aState != aLastState){     
     // If the ROTARY_PINB state is different to the ROTARY_PINA state, that means the encoder is rotating clockwise
     if (digitalRead(ROTARY_PINB) != aState) { 
       counter ++;
     } else {
       counter --;
     }
   } 
   aLastState = aState; // Updates the previous state of the ROTARY_PINA with the current state
}

void determineOption(){
  int index_cur = abs(counter/counter_gap%optionCount);
  if(index_cur != i){
    option = options[index_cur];
    i = index_cur;
    Serial.print(option);
  }
}

void flashCursor(){
  if(millis()-oled_cursor.last_blink>=oled_cursor.period){
    oled_cursor.last_blink = millis();
    Serial.println("flash");
    // draw mode
    if(oled_cursor.on_of){
      display.fillRoundRect(oled_cursor.x, oled_cursor.y, 
        oled_cursor.width, oled_cursor.height, 2, WHITE);
    }else{
      display.fillRoundRect(oled_cursor.x, oled_cursor.y, 
        oled_cursor.width, oled_cursor.height, 2, BLACK);
    }
    oled_cursor.on_of = !oled_cursor.on_of;
    display.display(); 
  }
}

void loop() { 
  updateCounter();
  determineOption();
  flashCursor();
 }