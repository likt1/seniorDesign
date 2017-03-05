// PRU-ICSS program to flash a LED on P9_27 (pru0_pru_r30_5)

.origin 0               // start of program in PRU memory
.entrypoint START       // program entry point

#define INS_PER_US 200        // 5ns per instruction
#define INS_PER_DELAY_LOOP 2  // two instructions per delay loop

// Set up a 50ms delay
#define DELAY 50 * 1000 * (INS_PER_US / INS_PER_DELAY_LOOP)
#define PRU0_ARM_INTERRUPT 19 // allows notification of program compl

#define tmp0 r1

#define locals r6

START:
  MOV   locals, 0       // local values at pos 0
  LBBO  tmp0, locals, 0, 8 // grab first unsigned int and store in r1
  LSL   tmp0, tmp0, 1   // shift left
  SBBO  tmp0, locals, 0x8, 8 // store shift after val
LEDON:
  SET   R30.T5          // set the output pin (LED on)
  MOV   r0, DELAY       // store the length of the delay in reg0

DELAYON:
  SUB   R0, R0, 1       // decrement REG0 by 1
  QBNE  DELAYON, R0, 0  // loop to DELAYON, unless REG0 = 0

LEDOFF:
  CLR   R30.T5          // clear the output bin (LED off)
  MOV   R0, DELAY       // reset REG0 to the length of delay

DELAYOFF:
  SUB   R0, R0, 1       // dec REG0 by 1
  QBNE  DELAYOFF, R0, 0 // loop to DELAYOFF, unless REG0 = 0
  
// Notify flash to ARM
NOTIFY:  
  MOV   R31.B0, PRU0_ARM_INTERRUPT+16 // fire interrupt
  WBS   R31.T30         // wait for response from ARM
  
  // TODO check parameters

  QBA   LEDON           // resume
