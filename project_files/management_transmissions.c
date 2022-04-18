/*
 * management_transmissions.c
 *
 *  Created on: 18 avr. 2022
 *      Author: APrap
 */

//defines used for transmission
#include "management_movement.h"

#define CORRIDOR							1	// |  |

#define CROSSING_L_RIGHT					2	//  -----
#define CROSSING_L_RIGHT_FIRE				3	// |   __
												// |  |

#define CROSSING_L_LEFT						4	// -----
#define CROSSING_L_LEFT_FIRE				5	// __   |
												//   |  |

#define CROSSING_T							6	// ----------
#define CROSSING_T_FIRE_RIGHT				7	// ___    ___
#define CROSSING_T_FIRE_LEFT				8	//    |  |
#define CROSSING_T_FIRE_ALL					9

#define CROSSING_T_LEFT						10	//     |  |
#define CROSSING_T_LEFT_FIRE_LEFT			11	// ----   |
#define CROSSING_T_LEFT_FIRE_UP				12	// ____   |
#define CROSSING_T_LEFT_FIRE_ALL			13	//     |  |

#define CROSSING_T_RIGHT					14	// |  |
#define CROSSING_T_RIGHT_FIRE_RIGHT			15	// |   ----
#define CROSSING_T_RIGHT_FIRE_UP			16	// |   ____
#define CROSSING_T_RIGHT_FIRE_ALL			17	// |  |

#define CROSSING_X							18	//    |  |
#define CROSSING_X_FIRE_RIGHT				19	// ---    ---
#define CROSSING_X_FIRE_UP					20	// ___    ___
#define CROSSING_X_FIRE_LEFT				21	//    |  |
#define CROSSING_X_FIRE_RIGHT_UP			22
#define CROSSING_X_FIRE_LEFT_UP				23
#define CROSSING_X_FIRE_RIGHT_LEFT			24
#define CROSSING_X_FIRE_ALL					25

#define DEAD_END							26	//  __
#define DEAD_END_FIRE						27	// |  |

void store_buffer(int map_configuration){

}

void Transmission_mapping(uint8_t movement_state, bool opening_right, bool openig_front, bool opening_left,
						bool opening_left, bool opening_left, bool fire_right, bool fire_front, bool fire_left)
{
	if(movement_state == MOVING) store_buffer(CORRIDOR);
	else
	{
		if if if ...
	}


}

static THD_WORKING_AREA(waThdTransmissions, 256);
static THD_FUNCTION(Transmissions, arg) {

    chRegSetThreadName(__FUNCTION__);

    systime_t time;

    while(1){

    	time = chVTGetSystemTime();



//    	//Selector control
//    	if(get_selector() == 0) movement_state = STOP;
//    	else if(movement_state == STOP){
//    		chThdSleepMilliseconds(500);
//    		movement_state = LEAVING_INTERSECTION;
//    	}


    	switch(get_movement_state()){


       	case STOP: 					break;


    	case MOVING: 				break;


    	case REACHING_INTERSECTION: break;


    	case ROTATING: 				break;


    	case LEAVING_INTERSECTION:  break;

    	case FIRE_FIGHTING:			break;

    	default: break;
    	}

    	chThdSleepUntilWindowed(time, time + MS2ST(1000));

    }
}

void management_transmissions_start(void){
	   chThdCreateStatic(waThdTransmissions, sizeof(waThdTransmissions), NORMALPRIO, Transmissions, NULL);
}
