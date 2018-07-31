# AutomataKit Stepper17

This is a motor driver for Nema 17 size Stepper Motors, which extends the [automatakit](https://gitlab.cba.mit.edu/jakeread/automatakit) architecture.

The board receives commands from an automatakit network and faithfully executes them, replying when complete. 

![board](/images/fab-front.jpg)

![board](/images/fab-back.jpg)

## Development Notes

See [circuit chatter](/circuit) and [firmware chatter](/embedded).

# Viable Commands

### Test

Keycode: **127**
 - to test networking, this will reply with a payload packet containing ```127, 12, 24, 48``` and will toggle an LED on the board

### Reset

Keycode: **128**
 - issues a software microcontroller reset
 - if the microcontroller is already hung up, this will not work

### Step Trapezoid

Keycode: **131**
Arguments: Steps to Make, int32_t | Entry Speed, steps/s, uint32_t | Acceleration Rate, steps/s/s, uint32_t | Acceleration Length, steps to accelerate for, uint32_t | Decceleration Length, steps to deccelerate after, uint32_t
Returns: Steps Made, int32_t | on step completion
 - the stepper will load this block into its stepping buffer, and if the buffer is empty will execute the trapezoid. on it's completion, the stepper will reply with an acknowledgement

### Step Wait

Keycode: **133**
Arguments: Steps to Make, int32_t | Entry Speed, steps/s, uint32_t | Acceleration Rate, steps/s/s, uint32_t | Acceleration Length, steps to accelerate for, uint32_t | Decceleration Length, steps to deccelerate after, uint32_t
Returns: Wait Made, uint8_t *24*
 - an identical block to the step trapezoid, but with a flag to avoid actually stepping. this is used in motion planning to insert a wait block for a motor with zero steps, to retain packet synchronization

# Reproducing This Work

All automatakit works are open source, and while we cannot sell you boards, if you have an interesting application, get in touch to ask about collaborating.

To reproduce boards and load code, see the document ['Reproducing Automatakit Work'](https://gitlab.cba.mit.edu/jakeread/automatakit/reproducing)