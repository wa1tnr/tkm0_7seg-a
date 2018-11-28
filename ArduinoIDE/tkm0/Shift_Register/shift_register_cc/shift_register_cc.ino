// 28 NOV 2018 09:02z

// finpaflik

#define DATENOW "Wed 28 Nov 2018  09:02z"
#include "config.h"
#include "src/periph/dotstar.h"

/*
Adafruit Arduino - Lesson 4. 8 LEDs and a Shift Register
Simon Monk mods: Christopher W Hafey, wa1tnr  27 November 2018
*/

// Trinket M0
int latchPin = 2;
int clockPin = 3;
int dataPin  = 4;

byte leds = 0;
byte uleds = 0;
byte pos = 15; // rightmost

byte slew = 5;

uint8_t ledval = 0;

void setup(void) {
    setup_dotstar();

    Serial.begin(9600);

    // while(!Serial) {

    for (int i = 4 ; i > 0 ; i--) {
        loop_dotstar(); // 4th iteration turns off dostar
    }
    Serial.print(DATENOW);

    pinMode(latchPin, OUTPUT);
    pinMode(dataPin,  OUTPUT);
    pinMode(clockPin, OUTPUT);
}

void _digitSelect(void) {
    uleds = pos;
    shiftOut(dataPin, clockPin, MSBFIRST, uleds);
}

void updateShiftRegister(void) {
    digitalWrite(latchPin, LOW);
    _digitSelect(); // digit 0 1 2 or 3 using 'pos' as the index

    uleds = leds;   // A-F 0-9 and a few other glyphs

    shiftOut(dataPin, clockPin, MSBFIRST, uleds); // paint the character's glyph!

    digitalWrite(latchPin, HIGH);
}

void blankleds(void) {
    leds = 0;
    updateShiftRegister();
}


void setleds(void) {
    leds = ledval;
    updateShiftRegister();
    if (!EXPOSE_DIGIT_PAINTING) {
        delay(1); // CRITICAL - must be a finite, non-zero delay here
    } else {
        delay(400);
    }
}

void flash_digit(void) {
    if (EXPOSE_DIGIT_PAINTING) {
        // delay(122); // to expose digit change
        delay(424); // to expose digit change
    }
    setleds();
    blankleds();
}

void in_column_zero(void) {
    for (int i = REPETITIONS ; i>0; i--) {
        pos = 15 ; flash_digit();
    }
}

void in_column_one(void) {
    for (int i = REPETITIONS ; i>0; i--) {
        pos = 22 ; flash_digit();
    }
}

void in_column_two(void) {
    for (int i = REPETITIONS ; i>0; i--) {
        pos = 27 ; flash_digit();
    }
}

void in_column_three(void) {
    for (int i = REPETITIONS ; i>0; i--) {
       pos = 29 ; flash_digit();
    }
}

void encode_zero(void) { // 0
    ledval = 1 + 2 + 4 + 8 + 16 + 32 +  0 +   0;
}

void encode_one(void) { // 1
    ledval = 0 + 2 + 4 + 0 +  0 +  0 +  0 +   0;
}

void encode_two(void) { // 2
    ledval = 1 + 2 + 0 + 8 + 16 +  0 + 64 +   0;
}

void encode_three(void) { // 3
    ledval = 1 + 2 + 4 + 8 +  0 +  0 + 64 +   0;
}

void encode_four(void) { // 4
    ledval = 0 + 2 + 4 + 0 +  0 + 32 + 64 +   0;
}

void encode_five(void) { // 5
    ledval = 1 + 0 + 4 + 8 +  0 + 32 + 64 +   0;
}

void encode_six(void) { // 6
    ledval = 1 + 0 + 4 + 8 + 16 + 32 + 64 +   0;
}

void encode_seven(void) { // 7
    ledval = 1 + 2 + 4 + 0 +  0 +  0 +  0 +   0;
}

void encode_eight(void) { // 8
    ledval = 1 + 2 + 4 + 8 + 16 + 32 + 64 +   0;
}

void encode_nine(void) { // 9
    ledval = 1 + 2 + 4 + 0 +  0 + 32 + 64 +   0;
}

void encode_ltr_l(void) { // L
    ledval = 0 + 0 + 0 + 8 + 16 + 32 +  0 +   0;
}

void encode_ltr_a(void) { // A -- the letter, A
    ledval = 1 + 2 + 4 + 16 + 32 + 64;
}

void encode_ltr_b(void) { // b
    ledval = 4 + 8 + 16 + 32 + 64;
}

void encode_ltr_c(void) { // C
    ledval = 1 + 8 + 16 + 32;
}

void encode_ltr_d(void) { // d
    ledval = 2 + 4 + 8 + 16 + 64;
}

void encode_ltr_e(void) { // E
    ledval = 1 + 8 + 16 + 32 + 64;
}

void encode_ltr_f(void) { // F
    ledval = 1 + 0 + 16 + 32 + 64;
}

void encode_ltr_blank(void) { // blank
    ledval = 0 ;
}


void loop(void) {
    blankleds();
    delay(40);
    ledval = 0;
    int i = 0;
    delay(1000);

    // message: '3223'
    for (int j = 2;  j>0; j--) {
        for (int k = DURATION; k>0; k--) {
            encode_three();  in_column_zero();   // print '3' in column '0'
            encode_two();    in_column_one();    // print '2' in column '1'
            encode_two();    in_column_two();    // print '2' in column '2'
            encode_three();  in_column_three();  // print '3' in column '3'
        }
    }

    delay(1000);

  // message:  'A824'
  for (int j = 2;  j>0; j--) {
      for (int k = DURATION; k>0; k--) {
          encode_four();   in_column_zero();
          encode_two();    in_column_one();
          encode_eight();  in_column_two();
          encode_ltr_a();      in_column_three();
      }
  }


  delay(1000);

  // message:  'LE  '
  for (int j = 2;  j>0; j--) {
      for (int k = DURATION; k>0; k--) {
          encode_ltr_blank();   in_column_zero();
          encode_ltr_blank();  in_column_one();
          encode_ltr_e();      in_column_two();
          encode_ltr_l();      in_column_three();
      }
  }


  delay(1000);

  // message: 'F0CA'

  for (int j = 2;  j>0; j--) {
      for (int k = DURATION; k>0; k--) {
          encode_ltr_a();     in_column_zero();
          encode_ltr_c();     in_column_one(); 
          encode_zero();     in_column_two();
          encode_ltr_f();     in_column_three();
      }
  }





  delay(1000);

  // message: 'CAFE'

  for (int j = 2;  j>0; j--) {
      for (int k = DURATION; k>0; k--) {
          encode_ltr_e();     in_column_zero();
          encode_ltr_f();     in_column_one();
          encode_ltr_a();     in_column_two();
          encode_ltr_c();     in_column_three();
      }
  }



  i = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128;
  i = 0;

  // hold display blank for a while:
  i = 128; ledval = i; in_column_zero(); blankleds();
      delay(111 + slew);

  // hold display blank for even a bit longer:
  i = 128; ledval = i; in_column_zero(); blankleds();
      delay(111 + slew);

}


/*
#define EXPOSE_DIGIT_PAINTING -1
*/
// #define EXPOSE_DIGIT_PAINTING  0
