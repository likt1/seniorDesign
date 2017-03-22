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
#define SHARED_RAM    0x10000

// Local offsets
#define ADDR          0x0
#define LENGTH        0x4
#define CAP_DELAY     0x8
#define TIMER         0xc
#define STOP_F        0x10
#define BUF1_F       0x14
#define BUF2_F       0x18

// Register allocations
#define adc_           r6
#define fifo0data      r7
#define samp_off       r8
#define samp_ind_off   r9
#define buff_samp_off  r17
#define local          r10
#define next_buf       r18

#define value          r11
#define channel        r12
#define samp_amt       r13
#define max_samp       r14
#define cap_delay      r15

#define tmp0           r1
#define tmp1           r2
#define tmp2           r3
#define tmp3           r4
#define tmp4           r5    // flag lookup tmp
#define tmp5           r16   // flag lookup tmp

// 1 word is 4 bytes

.origin 0               // start of program in PRU memory
.entrypoint START       // program entry point

START:
  LBCO r0, C4, 4, 4                 // Load in settings reg
  CLR  r0, r0, 4                    // Clear bit 4 in setting
  SBCO r0, C4, 4, 4                 // Restore
  
  MOV   adc_, ADC_BASE              // store ADC_BASE in adc_
  MOV   fifo0data, ADC_FIFO0DATA    // store ADC_FIFO0DATA in fifo0data
  MOV   local, ADDR                 // local vars exist at 0 mem loc
  MOV   next_buf, 1                 // start on buffer 1

  LBBO  buff_samp_off, local, ADDR, 4  // load buffer offset
  LBBO  cap_delay, local, CAP_DELAY, 4 // load capture delay

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
  MOV   samp_ind_off, 0             // reset offset
  MOV   samp_amt, 0                 // reset samp_amt sampled
  
  QBEQ  SET_BUFF2, next_buf, 2      // check if needs to go to buff 2
SET_BUFF1:
  LBBO  tmp4, local, BUF1_F, 4      // load buffer 1 entry flag and 
  QBEQ  SET_BUFF1, tmp4, 1          // don't leave until buffer 1 is ready
  LSL   next_buf, next_buf, 1       // set buffer 2 to next buffer
  MOV   samp_off, buff_samp_off     // load addr from stored samp off
  JMP   BEG_CAPTURE                 // begin sampling
SET_BUFF2:
  LBBO  tmp4, local, BUF2_F, 4      // load buffer 2 entry flag and 
  QBEQ  SET_BUFF2, tmp4, 1          // don't leave until buffer 2 is ready
  LSR   next_buf, next_buf, 1       // set buffer 1 to next buffer
  MOV   samp_off, SHARED_RAM        // move shared ram offset into sample offset
  
BEG_CAPTURE:
  QBNE  CAPTURE_DELAY, cap_delay, 0 // check delay

SAMPLE:
  LBBO  tmp4, local, STOP_F, 4      // look at stop flag
  QBNE  EXIT, tmp4, 1               // and exit if set
  
  MOV   tmp0, 0x1fe
  SBBO  tmp0, adc_, STEPCONFIG, 4   // write to STEPCONFIG to trigger cap

  // Inc values while waiting
  ADD   samp_amt, samp_amt, 1       // inc samp_amt sampled
  MOV   tmp1, 0xfff                 // init select reg for value

WAIT_FOR_FIFO0:
  LBBO  tmp2, adc_, FIFO0COUNT, 4   // load in FIFO0COUNT from adc_ loc
  QBNE  WAIT_FOR_FIFO0, tmp2, 8     // loop if value from FIFO0COUNT doesn't eq 8

READ_ALL_FIFO0:
  LBBO  value, fifo0data, 0, 4      // take value from fifo0data
  LSR   channel, value, 16          // extract channel from value
  AND   channel, channel, 0xf       // select last 4 bits from channel
  QBNE  READ_ALL_FIFO0, channel, 0  // only save channel 0
  AND   value, value, tmp1          // select last 12 bits from value
  LSL   value, value, 4             // shift left 4 bits to upscale to 16 bit
  SBBO  value, samp_off, samp_ind_off, 4 // store value into sample offset
  ADD   samp_ind_off, samp_ind_off, 2 // inc array offset value half to store halfwords

  LBBO  tmp0, local, LENGTH, 4      // grab max size
  QBNE  BEG_CAPTURE, samp_amt, tmp0 // if num samples gotten !eq max, loop

// Update and notify when buffer is filled
  MOV   tmp4, 1                     // move set bit into tmp4
  QBNE  UPD_BUF2, samp_off, buff_samp_off // if we are not on buffer 1 update buffer 2 flag
  SBBO  tmp4, local, BUF1_F, 4      // buffer 1 is filled
  JMP   NOTIFY
UPD_BUF2:
  SBBO  tmp4, local, BUF2_F, 4      // buffer 2 is filled
NOTIFY:
  MOV   R31.B0, PRU0_ARM_INT+16     // we are done with a buffer, fire interrupt to arm
  JMP   INIT_CAPTURE                // jump to start of capture cycle

CAPTURE_DELAY:
  MOV   tmp0, cap_delay
DELAY_LOOP:
  SUB   tmp0, tmp0, 1
  QBNE  DELAY_LOOP, tmp0, 0
  JMP   SAMPLE

// debug
QUIT:
  MOV   tmp4, value
  SBBO  tmp4, local, 0x4, 4        // offset of local should be beef now
  MOV   R31.B0, PRU0_ARM_INT+16     // fire interrupt
  WBS   R31.T30                     // wait for response from ARM
EXIT:
  HALT
