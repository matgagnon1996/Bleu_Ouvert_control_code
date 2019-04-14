#ifndef FSM_MAIN_H
#define FSM_MAIN_H

#include <stdint.h>


#define FSM_EXTRUDER_MODE			0
#define FSM_CRUSHER_MODE			1

struct controlStatus {
	//status
	volatile char* fsmCrusherStatus; // updated by fsm
	volatile char* fsmExtruderStatus; // updated by fsm
	volatile int fsmRequestedMode; // 0 = mode extrudeur, 1 = mode déchiqueteur (from interface)
	volatile int enslavementStarted; // 0 = not started, 1 = started (from interface)
	volatile int crusherIsBlocked; // 0 = not blocked, 1 = blocked (updated by fsm)
	volatile int moldIsFull;		// 0 = not full, 1 = full (updated by fsm_
	//current
	volatile double crusherCurrent; // update by sts thread
	volatile double extruderCurrent; // updated by sts trhead
	//temperature
	volatile double extruderTemperature; // updated by sts thread
	volatile double requestedTemperature; // from interface
	// weight
	volatile double convoyWeight; // updated by sts thread
	volatile double requestedWeight; // from interface
};

typedef struct controlStatus controlStatus_t;

// declaration
extern controlStatus_t *mainFsmStatus;

//function
void initMainFsm(void);

#endif
