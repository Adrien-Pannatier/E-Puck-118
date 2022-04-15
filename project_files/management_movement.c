/*
 * management_movement.c
 *
 *  Created on: 15 avr. 2022
 *      Author: APrap
 */

#include "ch.h"
#include "hal.h"
#include "management_movement.h"
#include "motors.h"

//provisoir
#include "leds.h"
#include "sensors/proximity.h"

static bool displacement = false;
static bool fire_detected = false;
static bool oppening_detected = false;
static bool dead_end = false;

//Thread displacement corridor
static THD_WORKING_AREA(waThdDisasterAreaDisplacement, 256);
static THD_FUNCTION(DisasterAreaDisplacement, arg) {

    chRegSetThreadName(__FUNCTION__);


    //initialisation thread motors
    motors_init();

    //Variables PI
    int32_t error = 0;
    static int32_t sum_errors = 0;
    int16_t correction = 0;

    uint16_t proximity_intensity_right = 0;
    uint16_t proximity_intensity_left = 0;

    while(1){

    	if(displacement)
    	{
    		//PI
    		error = proximity_intensity_left - proximity_intensity_right;

			//anti windup
			if(absolute_value_int32(sum_errors) < ANTI_WINDUP)	sum_errors += error;

			correction = KP * error + KI * sum_errors;

			//set motor speed
			right_motor_set_speed(DISPLACEMENT_SPEED + correction);
			left_motor_set_speed(DISPLACEMENT_SPEED - correction);

			chThdSleepMilliseconds(50);
    	}
    	else
    	{
    		right_motor_set_speed(SPEED_NUL);
    		left_motor_set_speed(SPEED_NUL);
    	}



    }
}

static THD_WORKING_AREA(waThdMovement, 256);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);


    //initialisation thread motors
    motors_init();

    while(1){

    	//if the robot is in journey arround the mase
    	if(get_selecto() == 1) displacement = true;
    	else displacement = false;

    	chThdSleepMilliseconds(100);
    }
}

void start_management_movement(void){
	   chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
	   chThdCreateStatic(waThdDisasterAreaDisplacement, sizeof(waThdDisasterAreaDisplacement), NORMALPRIO, DisasterAreaDisplacement, NULL);
}

uint32_t absolute_value_int32(int32_t value){
	if(value < 0) value = -value;
	return value;
}
