/*
 * asservissementTemperature.h
 *
 *  Created on: Apr. 2, 2019
 *      Author: mathieu
 */

#ifndef ASSERVISSEMENTTEMPERATURE_H_
#define ASSERVISSEMENTTEMPERATURE_H_

double calculateActionToApply(double);
void setTemperatureCommand(void);
double getTemperatureCommand(void);
void clearTemperatureCommand(void);
double voltageToDutyCycle(double);


#endif /* ASSERVISSEMENTTEMPERATURE_H_ */
