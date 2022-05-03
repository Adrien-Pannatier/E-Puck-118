/*
 * handle_fire.c
 *
 *  Created on: 21 avr. 2022
 *      Author: AdrienPannatier
 *
 *  Functions to control the alarm and the air blower module
 */

#include "handle_fire.h"
#include "leds.h"
#include "process_image.h"
#include "added_melodies.h"

bool check_for_fire(void){
	//turns on the camera, check then turns of
	start_image_processing();
	chThdSleepMilliseconds(500);
	stop_image_processing();

	if(get_line_position() == NOT_FOUND){
		return false;
	}
	else{
		return true;
	}
}

void deploy_antifire_measures(void){
	//activates the front led pin which turns on the air blower
	set_front_led(1);
//turns on the siren
	playAddedAlarm(PINPON,ML_SIMPLE_PLAY);
}

void stop_antifire_measures(void){
	//turns off the air blower
	set_front_led(0);
	//stops the melody
	//stopCurrentMelody();
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
