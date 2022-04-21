/*
 * management_movement.h
 *
 *  Created on: 15 avr. 2022
 *      Author: APrap
 */

#ifndef MANAGEMENT_MOVEMENT_H_
#define MANAGEMENT_MOVEMENT_H_

#include "ch.h"
#include "hal.h"

//Moving state
#define STOP							0												//No movement
#define MOVING							1												//Following a corridor
#define REACHING_INTERSECTION			2												//Reaching the middle of an intersection
#define ROTATING						3												//Choosing the next path and rotating in the intersection
#define LEAVING_INTERSECTION			4												//Regaining the corridor
#define FIRE_FIGHTING					5												//Taking actions against fire

#define NORTH							0
#define EAST							90
#define SOUTH							180
#define WEST							270

void management_movement_start(void);
bool get_fire_detected(void);
uint8_t get_movement_state(void);
uint8_t get_orientation(void);


#endif /* MANAGEMENT_MOVEMENT_H_ */
