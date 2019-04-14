/*
 * FSM_crusher.h
 *
 *  Created on: Mar. 31, 2019
 *      Author: mathieu
 */

#ifndef FSM_CRUSHER_H_
#define FSM_CRUSHER_H_


int updateCrusherFSM(void);
void startCrusherFSM(void);
void stopCrusherFSM(void);
void setCrusherFsmState(char* strValue);
void bourrageRoutineCrusherFSM(void);
void periodicReverseCrusherFSM(void);




#endif /* FSM_CRUSHER_H_ */
