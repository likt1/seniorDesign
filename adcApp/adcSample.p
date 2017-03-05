// PRU-ICSS program to flash a LED on P9_27 (pru0_pru_r30_5) until a button
// that is connected to P9_28 (pru0_pru_r31_3 is pressed.

.origin 0               // start of program in PRU memory
.entrypoint start       // program entry point

#define INS_PER_US 200        // 5ns per instruction
#define INS_PER_DELAY_LOOP 2  // two instructions per delay loop

// Set up a 50ms delay
#define DELAY 50 * 1000 * (INS_PER_US / INS_PER_DELAY_LOOP)
#define PRU0_R31_VEC_VALID 32 // allows notification of program compl
#define PRU_EVTOUT_0 3        // the event number that is sent back

start:
  set   r30.t5          // set the output pin (LED on)
  mov   r0, DELAY       // store the length of the delay in reg0

delayon:
  sub   r0, r0, 1       // decrement REG0 by 1
  qbne  delayon, r0, 0  // loop to DELAYON, unless REG0 = 0

ledoff:
  clr   r30.t5          // clear the output bin (LED off)
  mov   r0, DELAY       // reset REG0 to the length of delay

delayoff:
  sub   r0, r0, 1       // dec REG0 by 1
  qbne  delayoff, r0, 0 // loop to DELAYOFF, unless REG0 = 0
  
// Notify flash to ARM
notify:  
  mov   r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0 // fire interrupt
  wbs   r31.t30         // wait for response from ARM
  // TODO check parameters
  qba   start           // loop
