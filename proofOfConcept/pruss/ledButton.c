#include <stdio.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#define PRU_NUM 0				// using PRU0 for examples

void main (void) {
	printf("EBB PRU program start\n");

	// Init struct used by prussdrv_pruintc_intc
	// PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

	// Allocate and init mem
	int r = prussdrv_init();
	if (r != 0) {
		printf("Failed to init prussdrv driver");
		return;
	}
	r = prussdrv_open(PRU_NUM);
	if (r != 0) {
		printf("Failed to open PRU %d", PRU_NUM);
		prussdrv_exit();
		return;
	}

	// Map PRU's interrupts
	r = prussdrv_pruintc_init(&pruss_intc_initdata);
	if (r != 0) {
		printf("Failed to init interrupts");
		prussdrv_exit();
		return;
	}

	// Load and execute the PRU program on PRU
	//prussdrv_exec_program(PRU_NUM, "./ledButton.bin");

	// Wait for even compl from PRU, returns PRU_EVTOUT_0 num
	//int n = prussdrv_pru_wait_event(PRU_EVTOUT_0);
	int n = PRU_EVTOUT_0;
	printf("EBB PRU program completed, event number %d.\n", n);

	// Disable PRU and close memory mappings
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit();
}
