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
#define MUL_VAL			1324.6			//Interpolation MUL_VAL * x^(-0.79) [distance/intensity]
#define POWER			-0.79			//
#define TABLE_SIZE		600				//Size of the correspondence table
#define IR_NOISE		30				//Threshold for noise

//Private correspondence table
static uint8_t table_lin_IR[TABLE_SIZE];


/***************************INTERNAL FUNCTIONS************************************/

/**
 * @brief 			Fill up the correspondence table to linearize the IR sensors
 *
 */
void fill_correspondence_table(void)
{
    //file table lin IR sensors
    for(int i = 0; i < IR_NOISE; i++)
    {
    	table_lin_IR[i] = MAX_RANGE_IR;
    }
    for(int i = IR_NOISE; i < TABLE_SIZE; i++)
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
		if(intensity > (TABLE_SIZE - 1)) intensity = (TABLE_SIZE - 1);
		return(table_lin_IR[intensity]);
	}
	else return(NON_VALID);
}

/**************************END PUBLIC FUNCTIONS***********************************/

