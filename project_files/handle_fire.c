/*
 * handle_fire.c
 *
 *  Created on: 21 avr. 2022
 *      Author: AdrienPannatier
 */

#include "handle_fire.h"
#include "leds.h"
#include "process_image.h"
#include "added_melodies.h"

bool check_for_fire(void){
	//turns on the camera, check then turns of
	//MUST WAIT FOR CAMERA TO READ uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu

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
//	set_front_led(1);
//turns on the siren
	playAddedMelody(PINPON,ML_SIMPLE_PLAY);
}

void stop_antifire_measures(void){
//turns off the air blower
	set_front_led(0);
//stops the melody
	stopCurrentMelody();
}
