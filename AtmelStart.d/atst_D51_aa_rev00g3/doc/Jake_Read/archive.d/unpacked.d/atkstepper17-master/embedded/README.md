# AutomataKit Stepper17 Firmware Notes

Firmware for the board follows the [automatakit](https://gitlab.cba.mit.edu/jakeread/automatakit) architecture, and is available here for perusal.

Communication is handled asynchronously: bytes received on the UART are loaded into a ringbuffer, where they are later parsed in to packets. Parsing happens as oven as possible. 

Two hardware timers run step timing, one fires every time a step is to be taken, and another fires when acceleration needs to happen, the 2nd timer changes the period of the 1st. 

## Current Problems

- step-delay on first step causing issues?
- trinamic fiddling... need different sense resistors?
- check missing break; on wait case statement in handler?