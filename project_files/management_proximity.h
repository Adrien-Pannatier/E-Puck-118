/*
 * Proximity.h
 *
 *  Created on: 8 avr. 2022
 *      Author: APrap
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

#define MAX_INTENSITY		3500.0
#define NOISE_IR			50

void management_proximity_start(void);

//return the distance of the IR sensor
uint16_t get_intensity(uint8_t IR_number);

//COM
static void serial_start(void);


#endif /* PROXIMITY_H_ */
