/*
 * management_proximity.c
 *
 *  Created on: 8 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Functions and defines to use the IR captors, linearised
 */

#include <management_proximity.h>
#include <stm32f4xx.h>
#include "sensors/proximity.h"
#include "math.h"

//Private interpolation value for linearisation of IR value
#define MUL_VAL			1324.6
#define POWER			-0.79

//Private correspondence table
static uint8_t table_lin_IR[600];


/***************************INTERNAL FUNCTIONS************************************/

/**
 * @brief 			Fill up the correspondence table to linearize the IR sensors
 *
 */
void fill_correspondence_table(void)
{
    //file table lin IR sensors
    for(int i = 0; i < 30; i++)
    {
    	table_lin_IR[i] = MAX_RANGE_IR;
    }
    for(int i = 30; i < 600; i++)
    {
    	table_lin_IR[i] = (uint8_t) (MUL_VAL*powf((i), POWER));
    }
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

uint8_t get_distance_IR_mm(uint8_t IR_number)
{
	uint16_t intensity = get_calibrated_prox(IR_number);

	//Check if IR_number is a valid number, return the distance in mm from the transformation table
	if(IR_number <= 8)
	{
		if(intensity > 599) intensity = 599;
		return(table_lin_IR[intensity]);
	}
	else return(NON_VALID);
}

/**************************END PUBLIC FUNCTIONS***********************************/

