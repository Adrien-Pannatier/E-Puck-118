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
#include "management_proximity.h"

//provisoir
#include "leds.h"
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"

static bool displacement = false;
static bool moving_in_oppening = false;
static bool fire_detected = false;
static bool oppening_detected = false;
static bool dead_end = false;



static THD_WORKING_AREA(waThdMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);


    //initialisation thread motors
    motors_init();

    while(1){

    	//if the robot is in journey arround the mase
    	if(button_get_state() == 1)
    		{
    		displacement = true;
    		oppening_detected = false;
    		clear_leds();
    		}

    	//If oppening found move to the middle and show
    	if(oppening_detected) oppening_gestion();

    	chThdSleepMilliseconds(100);

    }
}

//Thread displacement corridor
static THD_WORKING_AREA(waThdDisasterAreaDisplacement, 512);
static THD_FUNCTION(DisasterAreaDisplacement, arg) {

    chRegSetThreadName(__FUNCTION__);

    	//Variables PI
   		int32_t error = 0;
   		static int32_t past_error = 0;
   		static int32_t sum_errors = 0;

   		int16_t correction = 0;

    while(1){

    	if(displacement)
    	{

			//PI
			error = get_intensity(IR3) - get_intensity(IR6);//)* 5.0 + get_intensity(IR2) - get_intensity(IR5) + get_intensity(IR4) - get_intensity(IR7);
			//
			//			//anti windup
			//			if(absolute_value_int32(sum_errors) < ANTI_WINDUP)	sum_errors += error;
			//
			correction = KP * error + KI * sum_errors + KD * (error - past_error);
			past_error = error;

			//set motor speed
			right_motor_set_speed(DISPLACEMENT_SPEED + correction);
			left_motor_set_speed(DISPLACEMENT_SPEED - correction);

			//check for oppenings
			//check_for_oppenings();
    	}
    	chThdSleepMilliseconds(50);

    }
}
void management_movement_start(void){
	   chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
	   chThdCreateStatic(waThdDisasterAreaDisplacement, sizeof(waThdDisasterAreaDisplacement), NORMALPRIO, DisasterAreaDisplacement, NULL);
}

void check_for_oppenings(void){

	if(get_calibrated_prox(IR3) <= NOISE_IR || get_calibrated_prox(IR6) <= NOISE_IR)
	{
		displacement = false;
		oppening_detected = true;
		moving_in_oppening = true;
	}

}

void oppening_gestion(void){

	static bool middle_reached = false;
	static bool init_counter = false;

	//set position to reach the middle
	if(moving_in_oppening)
	{
		if(!init_counter)
			{
			right_motor_set_pos(0);
			init_counter = true;

			right_motor_set_speed(DISPLACEMENT_SPEED);
			left_motor_set_speed(DISPLACEMENT_SPEED);
			}



	    if(right_motor_get_pos() >= DISTANCE_TO_REACH_THE_MIDDLE)
	    {
	    	//stop
	    	right_motor_set_speed(SPEED_NUL);
	    	left_motor_set_speed(SPEED_NUL);
	    	moving_in_oppening = false;
	    	middle_reached = true;
	    	init_counter = false;
	    }

	}

	//If position reached, show oppening
	if(middle_reached)
	{


		//show
		if(VL53L0X_get_dist_mm() >= VL53L0X_OPPENING) set_led(LED1, 1);
		if(get_calibrated_prox(IR3) <= NOISE_IR) set_led(LED3, 1);
		set_led(LED5, 1);
		if(get_calibrated_prox(IR6) <= NOISE_IR) set_led(LED7, 1);

		moving_in_oppening = false;
		middle_reached = false;
	 }
}

uint32_t absolute_value_int32(int32_t value){
	if(value < 0) value = -value;
	return value;
}
