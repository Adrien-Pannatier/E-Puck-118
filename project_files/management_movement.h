/*
 * management_movement.h
 *
 *  Created on: 15 avr. 2022
 *      Author: APrap
 */

#ifndef MANAGEMENT_MOVEMENT_H_
#define MANAGEMENT_MOVEMENT_H_

#define DISPLACEMENT_SPEED		500		//step/s
#define SPEED_NUL				0
#define ANTI_WINDUP				5000
#define KP						50
#define KI						0


void start_management_movement(void);
uint32_t absolute_value_int32(int32_t value);

#endif /* MANAGEMENT_MOVEMENT_H_ */
