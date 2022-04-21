/*
 * handle_fire.c
 *
 *  Created on: 21 avr. 2022
 *      Author: AdrienPannatier
 */

#include "handle_fire.h"
#include "leds.h"
#include <motors.h>

bool check_for_fire(void){

}

void deploy_antifire_measures(void){
//activates the front led pin which turns on the air blower
	set_front_led(1);


}

void stop_antifire_measures(void){
//turns off the air blower
	set_front_led(1);

}
