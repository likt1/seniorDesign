#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>

// ABOUT VALUES (Used limits.h to check):
// Our int can handle +-2147483647, so is 32bit
typedef unsigned int word;  // 32bit
typedef uint16_t halfword;  // 16bit
typedef uint8_t byte;       // 8bit

#define PRU_NUM 0 // Using PRU0
#define SAMPLE_RATE 44100 // Set sample rate
#define BUFFER_LENGTH 30 // DEBUG length is 30 sec
#define BUFFER_SIZE (SAMPLE_RATE*BUFFER_LENGTH) // Set buffer size
#define CONFIG_SIZE 10 // Config strings are 9 chars long + \0

#define PRU0MAP_LOC "/sys/class/uio/uio0/maps/map0/"

typedef char bool; // Define bool
#define true 1
#define false 0

struct configs {
  bool footSwitch;
  char compRotary[CONFIG_SIZE];
  char timeRotary[CONFIG_SIZE];
};

/* bit data structure:
0x00 0x00 00 00 00 | 07 addr
  08   00 00 00 00 | 0F offset
  10   00 00 00 00 | 17 length
  18   00 00 00 00 | 1F cap_delay
  20   00 00 00 00 | 27 timer
  28   00 00 00 00 | 2F flags
*/
struct locals {
  struct samples {
    word addr;  // address of DDR memory bank
    word offset;  // byte offset into local memory to capture for `scope mode
    word length;  // byte size of available DDR mem bank (non-zero triggers `scope capture)
  };

  word cap_delay;  // extra delay to control capture frequency

  word timer; // counts number of ADC reads
  word flags; // runtime flags
};

#endif
