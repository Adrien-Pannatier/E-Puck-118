/*
 * Proximity.h
 *
 *  Created on: 8 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Functions and defines to use the IR captors, linearised
 */

#ifndef PROXIMITY_H_
#define PROXIMITY_H_

#include "ch.h"
#include "hal.h"

#define IR1					0
#define IR2					1
#define IR3					2
#define IR4					3
#define IR5					4
#define IR6					5
#define IR7					6
#define IR8					7
#define NON_VALID			0

#define MAX_INTENSITY		4000
#define NOISE_IR			35
#define ERROR_IR_MM			10			//[mm]
#define MAX_RANGE_IR		120 		//[mm]

/**
 * @brief	Return the distance of a sensor to an object in mm
 *
 * @param IR_number     number of the IR sensor.
 * @retval uint8_t		The distance in mm.
 */
uint8_t get_distance_IR_mm(uint8_t IR_number);

#endif /* PROXIMITY_H_ */
