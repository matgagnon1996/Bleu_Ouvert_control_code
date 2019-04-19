/*
 * FSM_extruder.h
 *
 *  Created on: Mar. 31, 2019
 *      Author: mathieu
 */

#ifndef FSM_EXTRUDER_H_
#define FSM_EXTRUDER_H_

int updateExtruderFSM(void);
void startExtruderFSM(void);
void stopExtruderFSM(void);
void bourrageRoutineExtruderFSM(void);
void setExtruderFsmState(char*);
void periodicReverseExtruderFSM();
void openDitributorGateFSMExtruder(void);
void closeDitributorGateFSMExtruder(void);


#endif /* FSM_EXTRUDER_H_ */
