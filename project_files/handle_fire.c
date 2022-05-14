/*
 * handle_fire.c
 *
 *  Created on: 21 avr. 2022
 *      Author: Axel Praplan, Adrien Pannatier
 *
 *  Functions to control the alarm and the air blower module
 */

#include "handle_fire.h"
#include "leds.h"
#include "process_image.h"
#include "added_melodies.h"

/**************************PUBLIC FUNCTIONS***********************************/

bool check_for_fire(void){
	//turns on the camera, check then turns of
	start_image_processing();
	chThdSleepMilliseconds(500);
	stop_image_processing();

	if(get_fire_position() == NOT_FOUND){
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
	//playAddedAlarm(BASEBALL,ML_SIMPLE_PLAY);
}

void stop_antifire_measures(void){
	//turns off the air blower
	set_front_led(0);
	//stops the melody
	//stopCurrentMelody();
}

/**************************END PUBLIC FUNCTIONS***********************************/
