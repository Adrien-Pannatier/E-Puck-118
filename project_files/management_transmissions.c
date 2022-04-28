/*
 * management_transmissions.c
 *
 *  Created on: 18 avr. 2022
 *      Author: APrap
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

#define FACING_UP							21		//North
#define FACING_DOWN							22		//West
#define FACING_RIGHT						23		//East
#define FACING_LEFT							24		//South

#define BUFFER_SIZE							40
#define MAX_PTR_OFFSET						5
#define DATA_SIZE							1		//uint8_t


static uint16_t buffer_transmission[BUFFER_SIZE];
static uint8_t buffer_transmission_ptr_store = 0;
static uint8_t buffer_transmission_ptr_send = 0;
static uint8_t data_out[DATA_SIZE];

void store_buffer(uint8_t mapping_transmission){

	//store in the transmission buffer informations to send
	buffer_transmission[buffer_transmission_ptr_store] = mapping_transmission;

	//Incr and reset buffer ptr
	buffer_transmission_ptr_store++;
	if(buffer_transmission_ptr_store == BUFFER_SIZE) buffer_transmission_ptr_store = 0;

}

void send_orientation(uint16_t orientation){

		 if(orientation == NORTH) store_buffer(FACING_UP);
	else if(orientation == SOUTH) store_buffer(FACING_DOWN);
	else if(orientation == EAST) store_buffer(FACING_RIGHT);
	else if(orientation == WEST) store_buffer(FACING_LEFT);

}

void send_corridor(void){
	store_buffer(CORRIDOR);
}

void send_moving_in_intersection(void){
	store_buffer(MOVING_IN_INTERSECTION);
}

void send_crossing(bool opening_right, bool opening_front, bool opening_left){

		  if(!opening_left && !opening_right && !opening_front) store_buffer(DEAD_END);
	 else if(opening_left && !opening_right && !opening_front) store_buffer(CROSSING_L_LEFT);
	 else if(!opening_left && opening_right && !opening_front) store_buffer(CROSSING_L_RIGHT);
	 else if(opening_left && opening_right && !opening_front) store_buffer(CROSSING_T);
	 else if(opening_left && !opening_right && opening_front) store_buffer(CROSSING_T_LEFT);
	 else if(!opening_left && opening_right && opening_front) store_buffer(CROSSING_T_RIGHT);
	 else if(opening_left && opening_right && opening_front) store_buffer(CROSSING_X);

}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

void SendUint8ToComputer(uint8_t* data, uint16_t size)
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}


static THD_WORKING_AREA(waThdTransmissions, 256);
static THD_FUNCTION(Transmissions, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    //Realterm
    usb_start();
    serial_start();

    systime_t time;

    for(int i = 0; i < DATA_SIZE; i++){
    	data_out[i] = 0;
    }

    while(1){

    	time = chVTGetSystemTime();



//    	//Selector control
//    	if(get_selector() == 0) movement_state = STOP;
//    	else if(movement_state == STOP){
//    		chThdSleepMilliseconds(500);
//    		movement_state = LEAVING_INTERSECTION;
//    	}


    	//Transmission send

    	//Check if there is something to send in the buffer
    	if(buffer_transmission_ptr_send < buffer_transmission_ptr_store || (buffer_transmission_ptr_store < MAX_PTR_OFFSET && buffer_transmission_ptr_send > BUFFER_SIZE - MAX_PTR_OFFSET)){

			//Send to computer
    		data_out[0] = buffer_transmission[buffer_transmission_ptr_send];
    		SendUint8ToComputer(data_out, DATA_SIZE);

    		//chprintf((BaseSequentialStream *)&SD3, "Transmission = %.u \n\n\r", buffer_transmission[buffer_transmission_ptr_send]);


			//Incr and reset buffer ptr
			buffer_transmission_ptr_send++;
			if(buffer_transmission_ptr_send == BUFFER_SIZE) buffer_transmission_ptr_send = 0;
    	}



    	chThdSleepUntilWindowed(time, time + MS2ST(200));

    }
}

void management_transmissions_start(void){
	   chThdCreateStatic(waThdTransmissions, sizeof(waThdTransmissions), NORMALPRIO - 1, Transmissions, NULL);
}
