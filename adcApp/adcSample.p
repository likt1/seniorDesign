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
#define out_off   r9
#define local     r10

#define value     r11
#define channel   r12
#define local     r13
#define cap_delay r14

#define tmp0      r1
#define tmp1      r2
#define tmp2      r3
#define tmp3      r4
#define tmp4      r5

// 1 word is 4 bytes

START:
  MOV   adc_, ADC_BASE
  MOV   fifo0data, ADC_FIFO0DATA
  MOV   locals, 0

  LBBO  out_buff, locals, 0, 4
  LBBO  cap_delay, locals, 0x20, 4

  // Set up ADC
  // Disable
  LBBO  tmp0, adc_, CONTROL, 4
  MOV   tmp1, 0x1
  NOT   tmp1, tmp1
  AND   tmp0, tmp0, tmp1
  SBBO  tmp0, adc_, CONTROL, 4

  // Put ADC to full speed
  MOV   tmp0, 0
  SBBO  tmp0, adc_, SPEED, 4

  // Configure STEPCONFIG reg for all 8 channels
  MOV   tmp0, STEP1
  MOV   tmp1, 0
  MOV   tmp2, 0

FILL_STEPS:
  LSL   tmp3, tmp1, 19
  SBBO  tmp3, adc_, tmp0, 4
  ADD   tmp0, tmp0, 4
  SBBO  tmp2, adc_, tmp0, 4
  ADD   tmp1, tmp1, 1
  ADD   tmp0, tmp0, 4
  QBNE  FILL_STEPS, tmp1, 8

  // Enable ADC with desired modes and make STEPCONFIG writable
  LBBO  tmp0, adc_, CONTROL, 4
  OR    tmp0, tmp0, 0x7
  SBBO  tmp0, adc_, CONTROL, 4

CAPTURE:
  // check delay
  QBNE  CAPTURE_DELAY, cap_delay, 0

CPT_CONT:
  MOV   tmp0, 0x1fe
  SBBO  tmp0, adc_, STEPCONFIG, 4 // write to STEPCONFIG to trigger cap

  SUB   tmp0, tmp0, 4
  SBBO  tmp0, local, 0x14, 4 // HERERERERERERERERE
  

NOTIFY:  
  MOV   R31.B0, PRU0_ARM_INT+16 // fire interrupt
  WBS   R31.T30         // wait for response from ARM
  
  // TODO check parameters

  QBA   NOTIFY          // resume
