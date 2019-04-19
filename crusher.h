/*
 * crusher.h
 *
 *  Created on: Apr. 14, 2019
 *      Author: mathieu
 */

#ifndef CRUSHER_H_
#define CRUSHER_H_


void initCrusherMotor(void);
void stopCrusherMotor(int);
void startCrusherMotor(int);
void reverseCrusherMotor(int);

#define CRUSHER_FWD_SPEED				70 // %
#define CRUSHER_BWD_SPEED				30 // %


#endif /* CRUSHER_H_ */
