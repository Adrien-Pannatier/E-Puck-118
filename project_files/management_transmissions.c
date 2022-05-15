/*
 * management_transmissions.c
 *
 *  Created on: 18 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Thread and buffer to send labyrinth info to the computer
 *  via bluetooth
 */

//defines used for transmission
#include "management_movement.h"
#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

//Code used for transmission for mapping

#define NO_TRANSMISSION						0		//Transmission neutral

#define CORRIDOR							1		// |  |

#define MOVING_IN_INTERSECTION				2		//Going forward without drawing

#define FIRE_DETECTED						11		//Fire in front

#define CROSSING_T							12		// ----------
													// ___    ___
													//    |  |

#define CROSSING_T_RIGHT					13		// |  |
													// |   ----
													// |   ____
													// |  |

#define CROSSING_T_LEFT						14		//     |  |
													// ----   |
													// ____   |
													//     |  |

#define CROSSING_L_LEFT						15		// -----
													// __   |
													//   |  |

#define CROSSING_L_RIGHT					16		//  -----
													// |   __
													// |  |



#define CROSSING_X							17		//    |  |
													// ---    ---
													// ___    ___
													//    |  |


#define DEAD_END							18		//  __
													// |  |

#define CROSSING_UNKNOWN					19      //other

#define FACING_UP							21		//North
#define FACING_DOWN							22		//West
#define FACING_RIGHT						23		//East
#define FACING_LEFT							24		//South

#define BUFFER_SIZE							10
#define DATA_SIZE							1		//uint8_t
#define MAX_COUNTER							250

static uint8_t buffer_transmission[BUFFER_SIZE];
static uint8_t buffer_transmission_ptr_store = 0;
static uint8_t buffer_transmission_ptr_send = 0;
static uint16_t buffer_store_counter = 0;
static uint16_t buffer_send_counter = 0;
static bool reset_counter = false;

//Private functions
/**
 * @brief	Store a value in the transmission buffer
 * @details The function writes "START" before sending the informations.
 *
 * @param mapping_transmission     The parameter to store.
 */
void SendUint8ToComputer(uint8_t* data, uint16_t size);

/**
 * @brief			Send an array of Uint8 to sequential stream
 *
 *
 * @param data     	A pointer to the informations to send.
 * @param size    	The size of elements to send.
 */
void store_buffer(uint8_t mapping_transmission);

//Thread Transmission

static THD_WORKING_AREA(waThdTransmissions, 128);
static THD_FUNCTION(Transmissions, arg)
{
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1)
    {
    	//Transmission send

    	//Check if there is something to send in the buffer
    	if(buffer_send_counter < buffer_store_counter || reset_counter)
    	{
			//Send to computer
    		SendUint8ToComputer(&buffer_transmission[buffer_transmission_ptr_send], DATA_SIZE);

    		chprintf((BaseSequentialStream *)&SD3, "START %.u \n\r", buffer_transmission[buffer_transmission_ptr_send]);

			//Incr and reset buffer ptr
			buffer_transmission_ptr_send++;
			buffer_send_counter++;
			if(buffer_transmission_ptr_send == BUFFER_SIZE) buffer_transmission_ptr_send = 0;
			if(buffer_send_counter == MAX_COUNTER){
				buffer_send_counter = 0;
				reset_counter = false;
			}
    	}
    	chThdSleepMilliseconds(100);
    }
}

/***************************INTERNAL FUNCTIONS************************************/

void store_buffer(uint8_t mapping_transmission)
{
	//store in the transmission buffer informations to send
	buffer_transmission[buffer_transmission_ptr_store] = mapping_transmission;

	//Incr and reset buffer ptr
	buffer_transmission_ptr_store++;
	buffer_store_counter ++;
	if(buffer_transmission_ptr_store == BUFFER_SIZE) buffer_transmission_ptr_store = 0;
	if(buffer_store_counter == MAX_COUNTER){
	   buffer_store_counter = 0;
	   reset_counter = true;
	}

}

void SendUint8ToComputer(uint8_t* data, uint16_t size)
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

void send_orientation(uint16_t orientation)
{
		 if(orientation == NORTH) store_buffer(FACING_UP);
	else if(orientation == SOUTH) store_buffer(FACING_DOWN);
	else if(orientation == EAST) store_buffer(FACING_RIGHT);
	else if(orientation == WEST) store_buffer(FACING_LEFT);
}

void send_corridor(void)
{
	store_buffer(CORRIDOR);
}

void send_moving_in_intersection(void)
{
	store_buffer(MOVING_IN_INTERSECTION);
}

void send_crossing(bool opening_right, bool opening_front, bool opening_left)
{
		  if(!opening_left && !opening_right && !opening_front) store_buffer(DEAD_END);
	 else if(opening_left && !opening_right && !opening_front) store_buffer(CROSSING_L_LEFT);
	 else if(!opening_left && opening_right && !opening_front) store_buffer(CROSSING_L_RIGHT);
	 else if(opening_left && opening_right && !opening_front) store_buffer(CROSSING_T);
	 else if(opening_left && !opening_right && opening_front) store_buffer(CROSSING_T_LEFT);
	 else if(!opening_left && opening_right && opening_front) store_buffer(CROSSING_T_RIGHT);
	 else if(opening_left && opening_right && opening_front) store_buffer(CROSSING_X);
	 else store_buffer(CROSSING_UNKNOWN);

}

void send_fire(void)
{
	store_buffer(FIRE_DETECTED);
}

void management_transmissions_start(void)
{
	   chThdCreateStatic(waThdTransmissions, sizeof(waThdTransmissions), NORMALPRIO -1, Transmissions, NULL);
}

/**************************END PUBLIC FUNCTIONS***********************************/
