// 23 NOV 2018 02:21z beginning
// then came bronson
#define DATENOW "Tue 13 Mar 2018  06:04z"
#include "src/periph/dotstar.h"
extern void setup_dotstar();
extern void loop_dotstar();

/*
Adafruit Arduino - Lesson 4. 8 LEDs and a Shift Register
Simon Monk
mods: Christopher W Hafey, wa1tnr  13 March 2018
*/

// Trinket M0
int latchPin = 2;
int clockPin = 3;
int dataPin  = 4;

byte leds = 0;
byte uleds = 0;
byte pos = 15; // rightmost
// byte slew = 155; // 355 milliseconds
byte slew = 5; // 355 milliseconds

void setup(void) 
{
  setup_dotstar();

  Serial.begin(9600);

  // while(!Serial) {

  for (int i = 4 ; i > 0 ; i--) {
      loop_dotstar();
      // delay(500);
  }

  Serial.print(DATENOW);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);  
  pinMode(clockPin, OUTPUT);

}

void _digitSelect(void) {
   delay(12);
   uleds = pos; // uleds =   29; // valid: 15 rightmost or 2^0; 22 2^1, 27 and 29 or 0 (all four)
   shiftOut(dataPin, clockPin, MSBFIRST, uleds);
}

void _updateSR(void) {
   // delay(12);
   digitalWrite(latchPin, LOW);

   _digitSelect();
   uleds = leds; // sequential digit
   shiftOut(dataPin, clockPin, MSBFIRST, uleds);

   // delay(12);
   digitalWrite(latchPin, HIGH);
   delay(slew);
}

void updateShiftRegister(void) {
   // uleds = 0;     _updateSR();
   uleds = leds;  _updateSR();
}


#define DLYA 33
#define DLYB 100

#define DLYC 57
#define DLYD 744
uint8_t ledval = 0;
uint8_t oldval = 0;


void unfunc(uint8_t ledval) {
    int8_t i;
    for (i=7; i>=0; i--) {
        if (ledval &  (1 <<  (i)) ) {
            // digitalWrite(sid, HIGH);
            Serial.print("1");
        } else {
            // digitalWrite(sid, LOW);
            Serial.print("0");
        }
    }
    Serial.print(" ~stanza~ ");
}


void blankleds(void) {
    leds = 0;
    updateShiftRegister();
}


void setleds(void) {
    // leds = 0;
    // bitSet(leds, ledval); updateShiftRegister();

    leds = ledval; updateShiftRegister();
}


void glow(void) {
    blankleds();
    for (int i = 0; i < 125; i++) {
        setleds();
        blankleds();
// --------------------
        delay(2);
// --------------------
        // delay(1);
    }
}

void chordata(void) {
    blankleds();
    setleds();
}


void _puteye(void) {
  setleds(); 

  // delay(slew + slew);

  blankleds(); delay(1);
}

void outeye(void) {
  pos = 15 ; _puteye();
  pos = 22 ; _puteye();
  pos = 27 ; _puteye();
  pos = 29 ; _puteye();
}

void loop(void) 
{
  blankleds();
  delay(40);

  ledval = 0;
  oldval = 0;

  int i = 0;

  // 0 
  i = 1 + 2 + 4 + 8 + 16 + 32 +  0 +   0;
      ledval = i; outeye();

  // 1
  i = 0 + 2 + 4 + 0 +  0 +  0 +  0 +   0;
      ledval = i; outeye();

  // 2     // 1 2 8 16 64
  i = 1 + 2 + 0 + 8 + 16 +  0 + 64 +   0;
      ledval = i; outeye();

  // 3
  i = 1 + 2 + 4 + 8 +  0 +  0 + 64 +   0;
      ledval = i; outeye();

  // 4   //  2 4 32 64
  i = 0 + 2 + 4 + 0 +  0 + 32 + 64 +   0;
      ledval = i; outeye();

  // 5   // like 2
  i = 1 + 0 + 4 + 8 +  0 + 32 + 64 +   0;
      ledval = i; outeye();

  // 6
  i = 1 + 0 + 4 + 8 + 16 + 32 + 64 +   0;
      ledval = i; outeye();

  // 7
  i = 1 + 2 + 4 + 0 +  0 +  0 +  0 +   0;
      ledval = i; outeye();

  // 8
  i = 1 + 2 + 4 + 8 + 16 + 32 + 64 +   0;
      ledval = i; outeye();

  // 9
  i = 1 + 2 + 4 + 0 +  0 + 32 + 64 +   0;
      ledval = i; outeye();

  // A
  i = 1 + 2 + 4 + 16 + 32 + 64;
      ledval = i; outeye();

  // b
  i = 4 + 8 + 16 + 32 + 64;
      ledval = i; outeye();

  // C
  i = 1 + 8 + 16 + 32;
      ledval = i; outeye();

  // d
  i = 2 + 4 + 8 + 16 + 64;
      ledval = i; outeye();

  // E
  i = 1 + 8 + 16 + 32 + 64;
      ledval = i; outeye();

  // F
  i = 1 + 0 + 16 + 32 + 64;
      ledval = i; outeye();


  i = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128;
  i = 0;

  // hold display blank for a while:
  i = 128; ledval = i; outeye(); blankleds();
      delay(111 + slew);

  // hold display blank for even a bit longer:
  i = 128; ledval = i; outeye(); blankleds();
      delay(111 + slew);

}

