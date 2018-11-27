// 27 NOV 2018 14:23z

#define DATENOW "Tue 27 Nov Mar 2018  14:22z"
#include "config.h"
#include "src/periph/dotstar.h"
extern void setup_dotstar();
extern void loop_dotstar();

/*
Adafruit Arduino - Lesson 4. 8 LEDs and a Shift Register
Simon Monk
mods: Christopher W Hafey, wa1tnr  27 November 2018
*/

// Trinket M0
int latchPin = 2;
int clockPin = 3;
int dataPin  = 4;

byte leds = 0;
byte uleds = 0;
byte pos = 15; // rightmost

byte slew = 5;

void setup(void) 
{
  setup_dotstar();

  Serial.begin(9600);

  // while(!Serial) {

  for (int i = 4 ; i > 0 ; i--) {
      loop_dotstar();
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

void _updateSR(void) {
   digitalWrite(latchPin, LOW);

   _digitSelect();
   uleds = leds; // sequential digit
   shiftOut(dataPin, clockPin, MSBFIRST, uleds);

   digitalWrite(latchPin, HIGH);
}

void updateShiftRegister(void) {
   // uleds = 0;     _updateSR();
   uleds = leds;  _updateSR();
}

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
    leds = ledval; updateShiftRegister();
    delay(1); // CRITICAL - must be a finite, non-zero delay here
}

void _puteye(void) {
  if (EXPOSE_DIGIT_PAINTING) {
      delay(122); // to expose digit change
  }
  setleds();
  blankleds();
}

void _outeye_zero(void) {
  pos = 15 ; _puteye();

/*
  pos = 22 ; _puteye();
  pos = 27 ; _puteye();
  pos = 29 ; _puteye();
*/
}


void _outeye_one(void) {
/*
  pos = 15 ; _puteye();
*/

  pos = 22 ; _puteye();

/*
  pos = 27 ; _puteye();
  pos = 29 ; _puteye();
*/
}


void _outeye_two(void) {
/*
  pos = 15 ; _puteye();
  pos = 22 ; _puteye();
*/

  pos = 27 ; _puteye();

/*
  pos = 29 ; _puteye();
*/
}


void _outeye_three(void) {
/*
  pos = 15 ; _puteye();
  pos = 22 ; _puteye();
  pos = 27 ; _puteye();
*/
  pos = 29 ; _puteye();

}


void outeye_zero(void) {
    int duration = 2;
    for (int i = duration; i>0; i--) {
        _outeye_zero();
    }
}

void outeye_one(void) {
    int duration = 2;
    for (int i = duration; i>0; i--) {
        _outeye_one();
    }
}

void outeye_two(void) {
    int duration = 2;
    for (int i = duration; i>0; i--) {
        _outeye_two();
    }
}

void outeye_three(void) {
    int duration = 2;
    for (int i = duration; i>0; i--) {
        _outeye_three();
    }
}

void dig_zero(void) {
  int i = 0;

  // 0 
  i = 1 + 2 + 4 + 8 + 16 + 32 +  0 +   0;
      ledval = i;
}

void dig_one(void) {
  int i = 0;

  // 1
  i = 0 + 2 + 4 + 0 +  0 +  0 +  0 +   0;
      // ledval = i; outeye();
      ledval = i;
}

void dig_two(void) {
  int i = 0;

  // 2     // 1 2 8 16 64
  i = 1 + 2 + 0 + 8 + 16 +  0 + 64 +   0;
      ledval = i;
}


void dig_three(void) {
  int i = 0;

  // 3
  i = 1 + 2 + 4 + 8 +  0 +  0 + 64 +   0;
      ledval = i;
}


void dig_four(void) {
  int i = 0;

  // 4   //  2 4 32 64
  i = 0 + 2 + 4 + 0 +  0 + 32 + 64 +   0;
      ledval = i;
}

void dig_five(void) {
  int i = 0;

  // 5   // like 2
  i = 1 + 0 + 4 + 8 +  0 + 32 + 64 +   0;
      ledval = i;
}


void dig_six(void) {
  int i = 0;

  // 6
  i = 1 + 0 + 4 + 8 + 16 + 32 + 64 +   0;
      ledval = i;
}

void dig_seven(void) {
  int i = 0;
  // 7
  i = 1 + 2 + 4 + 0 +  0 +  0 +  0 +   0;
      ledval = i;
}



void dig_eight(void) {
  int i = 0;

  // 8
  i = 1 + 2 + 4 + 8 + 16 + 32 + 64 +   0;
      ledval = i;
}


void dig_nine(void) {
  int i = 0;
  // 9
  i = 1 + 2 + 4 + 0 +  0 + 32 + 64 +   0;
      ledval = i;
}

void ltr_a(void) { // the letter, A
  int i = 0;
  // A
  i = 1 + 2 + 4 + 16 + 32 + 64;
      ledval = i;
}

void ltr_b(void) {
  int i = 0;
  // b
  i = 4 + 8 + 16 + 32 + 64;
      ledval = i;
}

void ltr_c(void) {
  int i = 0;
  // C
  i = 1 + 8 + 16 + 32;
      ledval = i;
}

void ltr_d(void) {
  int i = 0;
  // d
  i = 2 + 4 + 8 + 16 + 64;
      ledval = i;
}

void ltr_e(void) {
  int i = 0;
  // E
  i = 1 + 8 + 16 + 32 + 64;
      ledval = i;
}

void ltr_f(void) {
  int i = 0;
  // F
  i = 1 + 0 + 16 + 32 + 64;
      ledval = i;
}



void loop(void) 
{
  blankleds();
  delay(40);

  ledval = 0;
  oldval = 0;

  int i = 0;


  delay(1000);

  // message: '3223'
  for (int j = 2;  j>0; j--) { // for loops provide duration
      for (int k = 255; k>0; k--) {
          dig_three(); outeye_zero();
          dig_two(); outeye_one();
          dig_two(); outeye_two();
          dig_three(); outeye_three();
      }
  }

  delay(1000);

  // message:  '45A8'
  for (int j = 2;  j>0; j--) {
      for (int k = 255; k>0; k--) {
          dig_eight(); outeye_zero();
          ltr_a();     outeye_one();
          dig_five();  outeye_two();
          dig_four();  outeye_three();
      }
  }

  delay(1000);

  // message: 'CAFE'

  for (int j = 2;  j>0; j--) {
      for (int k = 255; k>0; k--) {
          ltr_e();     outeye_zero();
          ltr_f();     outeye_one();
          ltr_a();     outeye_two();
          ltr_c();     outeye_three();
      }
  }



  i = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128;
  i = 0;

  // hold display blank for a while:
  i = 128; ledval = i; outeye_zero(); blankleds();
      delay(111 + slew);

  // hold display blank for even a bit longer:
  i = 128; ledval = i; outeye_zero(); blankleds();
      delay(111 + slew);

}


/*
#define EXPOSE_DIGIT_PAINTING -1
*/
#define EXPOSE_DIGIT_PAINTING  0
