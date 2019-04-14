/*
 * FSM_main.c
 *
 *  Created on: Mar. 31, 2019
 *      Author: mathieu
 */

#include "FSM_main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

controlStatus_t *mainFsmStatus = 0;

void initMainFsm()
{
	// init status struct
	mainFsmStatus = (controlStatus_t*)malloc(sizeof(controlStatus_t));

	// init struct field
	mainFsmStatus->fsmCrusherStatus = (char*)malloc(50);
	mainFsmStatus->fsmExtruderStatus = (char*)malloc(50);
	memcpy(mainFsmStatus->fsmCrusherStatus, "En attente", strlen("En attente"));
	memcpy(mainFsmStatus->fsmExtruderStatus, "En attente", strlen("En attente"));
	mainFsmStatus->fsmRequestedMode = 0;
	mainFsmStatus->enslavementStarted = 0;
	mainFsmStatus->crusherIsBlocked = 0;
	mainFsmStatus->moldIsFull = 0;
	mainFsmStatus->crusherCurrent = 0.0;
	mainFsmStatus->extruderCurrent = 0.0;
	mainFsmStatus->extruderTemperature = 0.0;
	mainFsmStatus->requestedTemperature = 0.0;
	mainFsmStatus->convoyWeight = 0.0;
	mainFsmStatus->requestedWeight = 0.0;
}


