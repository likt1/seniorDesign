// PRU-ICSS program to flash a LED on P9_28 (pru0_pru_r30_3)

#define PRU0_ARM_INT  19 // allows notification of program compl

#define ADC_BASE      0x44e0d000
#define ADC_FIFO0DATA (ADC_BASE + 0x0100)
#define CONTROL       0x0040
#define SPEED         0x004c
#define STEP1         0x0064
#define DELAY1        0x0068
#define STATUS        0x0044
#define STEPCONFIG    0x0054
#define FIFO0COUNT    0x00e4

.origin 0               // start of program in PRU memory
.entrypoint START       // program entry point

// Register allocations
#define adc_      r6
#define fifo0data r7
#define out_buff  r8
#define locals    r9

#define value     r10
#define channel   r11
#define ema       r12
#define encoders  r13
#define cap_delay r14

#define tmp0      r1
#define tmp1      r2
#define tmp2      r3
#define tmp3      r4
#define tmp4      r5

// 1 word is 4 bytes

START:
  MOV adc_, ADC_BASE
	MOV fifo0data, ADC_FIFO0DATA
	MOV locals, 0
  
  
  
LEDON:
  SET   R30.T3          // set the output pin (LED on)
  MOV   r0, DELAY       // store the length of the delay in reg0

DELAYON:
  SUB   R0, R0, 1       // decrement REG0 by 1
  QBNE  DELAYON, R0, 0  // loop to DELAYON, unless REG0 = 0

LEDOFF:
  CLR   R30.T3          // clear the output bin (LED off)
  MOV   R0, DELAY       // reset REG0 to the length of delay

DELAYOFF:
  SUB   R0, R0, 1       // dec REG0 by 1
  QBNE  DELAYOFF, R0, 0 // loop to DELAYOFF, unless REG0 = 0
  
// Notify flash to ARM
NOTIFY:  
  MOV   R31.B0, PRU0_ARM_INT+16 // fire interrupt
  WBS   R31.T30         // wait for response from ARM
  
  // TODO check parameters

  QBA   LEDON           // resume
