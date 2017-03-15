// adc sampling PRU loop

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

// Register allocations
#define adc_      r6
#define fifo0data r7
#define out_buff  r8
#define out_off   r9
#define local     r10

#define value     r11
#define channel   r12
#define length    r13
#define cap_delay r14

#define tmp0      r1
#define tmp1      r2
#define tmp2      r3
#define tmp3      r4
#define tmp4      r5

// 1 word is 4 bytes

.origin 0               // start of program in PRU memory
.entrypoint START       // program entry point

START:
  LBCO r0, C4, 4, 4                 // Load in settings reg
  CLR  r0, r0, 4                    // Clear bit 4 in setting
  SBCO r0, C4, 4, 4                 // Restore

  MOV   adc_, ADC_BASE              // store ADC_BASE in adc_
  MOV   fifo0data, ADC_FIFO0DATA    // store ADC_FIFO0DATA in fifo0data
  MOV   local, 0                    // local vars exist at 0 mem loc

  //LBBO tmp0, local, 0x14, 4         // eyecatcher check
  //MOV  tmp1, 0xbeef1965
  //QBNE QUIT, tmp0, tmp1

  LBBO  out_buff, local, 0, 4       // word addr in locals
  LBBO  cap_delay, local, 0xc, 4    // word cap_delay in locals

  // Set up ADC
  // Disable first
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

INIT_CAPTURE:
  MOV   out_off, 0                  // reset offset
  MOV   length, 0                   // reset length sampled

BEG_CAPTURE:
  QBNE  CAPTURE_DELAY, cap_delay, 0 // check delay

SAMPLE:
  MOV   tmp0, 0x1fe
  SBBO  tmp0, adc_, STEPCONFIG, 4   // write to STEPCONFIG to trigger cap

  // Inc values while waiting
  ADD   length, length, 1           // inc length sampled
  MOV   tmp0, out_off               // inc out_offset while
  ADD   out_off, out_off, 2         //   we wait
  MOV   tmp1, 0xfff                 // init select reg for value

WAIT_FOR_FIFO0:
  LBBO  tmp2, adc_, FIFO0COUNT, 4   // load in FIFO0COUNT from adc_ loc
  QBNE  WAIT_FOR_FIFO0, tmp2, 8     // loop if value from FIFO0COUNT doesn't eq 8

READ_ALL_FIFO0:
  LBBO  value, fifo0data, 0, 4      // take value from fifo0data
  LSR   channel, value, 16          // extract channel from value
  AND   channel, channel, 0xf       // select last 4 bits from channel
  AND   value, value, tmp1          // select last 12 bits from value
  SBBO  value, out_buff, tmp0, 2    // store value into out_buffer

  LBBO  tmp0, local, 0x08, 4        // grab max size
  QBNE  BEG_CAPTURE, length, tmp0   // if num samples gotten !eq max, loop

NOTIFY:  
  MOV   R31.B0, PRU0_ARM_INT+16     // fire interrupt
  WBS   R31.T30                     // wait for response from ARM
  
  // TODO check parameters

  QBA   INIT_CAPTURE                // resume

CAPTURE_DELAY:
  MOV   tmp0, cap_delay
DELAY_LOOP:
  SUB   tmp0, tmp0, 1
  QBNE  DELAY_LOOP, tmp0, 0
  JMP   SAMPLE

QUIT:
// debug
  MOV   tmp4, 0xbeef0010
  SBBO  tmp4, local, 0x4, 4        // offset of local should be beef now
  MOV   R31.B0, PRU0_ARM_INT+16     // fire interrupt
  WBS   R31.T30                     // wait for response from ARM
  HALT
// debug
