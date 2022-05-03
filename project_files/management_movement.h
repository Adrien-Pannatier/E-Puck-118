/*
 * management_movement.h
 *
 *  Created on: 15 avr. 2022
 *      Author: APrap
 *
 *  Functions and defines to manage the state of the robot and the
 *  way it moves and interacts with its surroundings
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
#define SEARCHING_FIRE					6												//searching if fire

#define NORTH							0
#define EAST							90
#define SOUTH							180
#define WEST							270

/**
 * @brief Activates the movement thread
 *
 */
void management_movement_start(void);

/**
 * @brief 			get if there is a fire or not
 *
 * @retval true		if there is a fire.
 * @retval false	if there is no fire.
 */
bool get_fire_detected(void);

/**
 * @brief 			get the current state of the robot
 *
 * @return			int value of the state (see the moving states above)
 */
uint8_t get_movement_state(void);

/**
 * @brief 			get the current orientation of the robot
 *
 * @return			int value of the orientation (see the orientations above)
 */
uint8_t get_orientation(void);


#endif /* MANAGEMENT_MOVEMENT_H_ */
