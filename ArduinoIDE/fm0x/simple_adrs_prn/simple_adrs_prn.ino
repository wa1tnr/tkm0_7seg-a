/* simple_adrs_prn.ino */
// Feather M0 Express - Arduino IDE
// 09 OCT 2018  22:44 UTC

// ram address:
// #define START_ADDRESS 0x200003BA

// internal flashROM address:
#define START_ADDRESS 0x1d20

// how much output - in units of 16 byte lines:
#define LINES 8

// p is an integer and is assigned a value that is an address:
int p = START_ADDRESS;

// generic print buffer to print to the USB serial port:
char buffer[128];

// simple putch() as used in the C language:
void putch(char ch) {
    Serial.print(ch);
}

int dump_16_bytes(void) {
    char *ram;
    ram = (char *) p; // not used immediately - see Line 43, below

    sprintf(buffer, "\n%4X: ", p); // print an integer 'p' as a formatted string,
                                   // to a string buffer 'buffer'

    Serial.print(buffer); // print the formatted string to the serial port

    int count = -1;
    for (int i = 0; i < 16; i++) {
        count++;
        if (i == 8)
            putch('\040'); // extra padding after eighth byte is printed
        if (count > 3) {
            count = (count & 0x03); // pad every four bytes ..
            putch('\040');          // with a space char, 0x20 (octal: \040)
        }
        char c = *ram++;      // c now holds the character stored at address p

                              // see Line 27, above, for initial value of *ram

// ----------------------------------------------------------------------------------------
// Exercise   - modify  *ram  so that it does not increment:
//
//      char c = *ram;        // why are we allowed to use  *ram++  here? What does it do?
// ----------------------------------------------------------------------------------------

        sprintf(buffer, " %02X", (c & 0xff)); // format c as two hexadecimal digits (in ASCII)
        Serial.print(buffer);
        // Serial.print("DebugFOO"); // random foo to prove where/when this gets executed
    }
    ram = (char *) p; // see Line 27 - does the exact same thing, but in the present context

    putch('\040'); putch('\040'); putch('\040'); // three spaces

    // print out the ASCII equivalents, in the rightmost 16 columns:
    for (int i = 0; i < 16; i++) {
        buffer[0] = (uint32_t) * ram++;
        if (buffer[0] > 0x7e || buffer[0] < ' ')
            buffer[0] = (uint32_t) '.'; // use a dot for non-printing characters
        buffer[1] = '\0';
        Serial.print(buffer);
    }
    // increment address in memory by 16:
    return p + 16;
}

void setup (void) {
    Serial.begin(9600);
    while (!Serial) { } // await a connection
    delay(2000); // after connect, wait 2 sec

    Serial.println("here is.");

    for (int index = LINES; index > 0; index--) { // dump three lines 16 bytes/line
        // p has an initial value the first time through this loop
        // p is an address in RAM or ROM
        p = dump_16_bytes();
    }
}

void loop (void) {
    while (-1) { }
    Serial.println("Escaped the while()\r\n");
}

