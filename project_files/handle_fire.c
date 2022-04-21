/*
 * handle_fire.c
 *
 *  Created on: 21 avr. 2022
 *      Author: AdrienPannatier
 */

#include "handle_fire.h"
#include "leds.h"
#include <motors.h>
#include "process_image.h"

//#define WIGGLE_SPEED				300

//internal functions
//bool wiggle(void);

bool check_for_fire(void){
	uint16_t position = NOT_FOUND;
	start_image_processing();
	position = get_peak_position();
	if(position != NOT_FOUND){
		stop_image_processing();
		return true;
	}
	else{
		stop_image_processing();
		return false;
	}
}

void deploy_antifire_measures(void){
//activates the front led pin which turns on the air blower
	set_front_led(1);
}

void stop_antifire_measures(void){
//turns off the air blower
	set_front_led(0);
}

//bool wiggle(void){
//	static bool rotation_R = false;
//	static bool rotation_L = false;
//	static bool return_to_centre = false;
//	if(rotation_R == false && rotation_L == false && return_to_centre == false){
//		rotation_R = rotate(10);
//		return false;
//	}
//	else if(rotation_R == true && rotation_L == false && return_to_centre == false){
//		rotation_L = rotate(-20);
//		return false;
//	}
//	else if(rotation_R == true && rotation_L == true && return_to_centre == false){
//		return_to_centre = rotate(10);
//		return false;
//	}
//	else if(rotation_R == true && rotation_L == true && return_to_centre == true){
//		return_to_centre = false;
//		rotation_R = false;
//		rotation_L = false;
//		return true;
//	}
//}
