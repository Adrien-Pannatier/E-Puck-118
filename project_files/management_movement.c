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

//provisoir pour debug
#include "leds.h"
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"
#include "chprintf.h"

static uint8_t movement_state = 0;
static bool displacement = false;
static bool moving_in_oppening = false;
static bool fire_detected = false;
static bool oppening_detected = false;
static bool dead_end = false;

//provisoir pour tunning
static float Kd_tun = 1.65;
static float Kp_tun = 0.06;
const float incr = 0.005;


static THD_WORKING_AREA(waThdMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);

    //Thread motors init
    motors_init();

    while(1){


    	//reset movement
    	if(button_get_state() == 1)
    	{
    		clear_leds();
    		movement_state = MOVING;
    	}

    	switch(movement_state){
    	case STOP: stop_movement(); break;
    	case MOVING: check_for_oppenings(); movement_regulation(); break;
    	case REACHING_INTERSECTION: moving_in_intersection(); break;
    	case ROTATING: break;
    	case LEAVING_INTERSECTION: break;
    	case FIRE_FIGHTING: break;
    	default: movement_state = STOP; break;
    	}

    	chThdSleepMilliseconds(50);

    }
}

////Thread displacement corridor
//static THD_WORKING_AREA(waThdDisasterAreaDisplacement, 512);
//static THD_FUNCTION(DisasterAreaDisplacement, arg) {
//
//    chRegSetThreadName(__FUNCTION__);
//
//    	//Variables PD
//   		int32_t error = 0;
//   		static int32_t past_error = 0;
////   		static int32_t sum_errors = 0;
//
//   		int16_t correction = 0;
//
//    while(1){
//
//    	if(movement_state == MOVING)
//    	{
//
//			//PD
//			error = get_intensity(IR3) - get_intensity(IR6);//)* 5.0 + get_intensity(IR2) - get_intensity(IR5) + get_intensity(IR4) - get_intensity(IR7);
//			//
//			//			//anti windup
//			//			if(absolute_value_int32(sum_errors) < ANTI_WINDUP)	sum_errors += error;
//			//
//			correction = Kp_tun * error + Kd_tun * (error - past_error);
//			past_error = error;
//
//			//set motor speed
//			right_motor_set_speed(DISPLACEMENT_SPEED + correction);
//			left_motor_set_speed(DISPLACEMENT_SPEED - correction);
//
//			//check for oppenings
//			check_for_oppenings();
//    	}
//    	chThdSleepMilliseconds(50);
//
//    }
//}


void management_movement_start(void){
	   chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
//	   chThdCreateStatic(waThdDisasterAreaDisplacement, sizeof(waThdDisasterAreaDisplacement), NORMALPRIO, DisasterAreaDisplacement, NULL);
}

bool get_fire_detected(void){
	return(fire_detected);
}


//Movement related functions

void stop_movement(void){
	right_motor_set_speed(SPEED_NUL);
	left_motor_set_speed(SPEED_NUL);
	return;
}

void movement_regulation(void)
{
	//Variables PD
	int32_t error = 0;
	static int32_t past_error = 0;
	int16_t correction = 0;

	//PD
	error = get_intensity(IR3) - get_intensity(IR6);//)* 5.0 + get_intensity(IR2) - get_intensity(IR5) + get_intensity(IR4) - get_intensity(IR7);
	//
	//			//anti windup
	//			if(absolute_value_int32(sum_errors) < ANTI_WINDUP)	sum_errors += error;
	//
	correction = Kp_tun * error + Kd_tun * (error - past_error);
	past_error = error;

	//set motor speed
	right_motor_set_speed(DISPLACEMENT_SPEED + correction);
	left_motor_set_speed(DISPLACEMENT_SPEED - correction);

}


void check_for_oppenings(void){

	//if oppening detected
	if(get_calibrated_prox(IR3) <= NOISE_IR || get_calibrated_prox(IR6) <= NOISE_IR)
	{
		movement_state = REACHING_INTERSECTION;
	}

}

void moving_in_intersection(void){

	static bool middle_reached = false;
	static bool init_counter = false;


	//init the counter only one time and start moving forward
	if(!init_counter)
	{
		right_motor_set_pos(0);
		init_counter = true;

		right_motor_set_speed(DISPLACEMENT_SPEED);
		left_motor_set_speed(DISPLACEMENT_SPEED);
	}


	//stop when the middle is reached
	if(right_motor_get_pos() >= DISTANCE_TO_REACH_THE_MIDDLE)
	{
	    stop_movement();
	    init_counter = false;
	    movement_state = STOP;

	    middle_reached = true;
	}


	//Developpement
	if(middle_reached)
	{


		//show
		if(VL53L0X_get_dist_mm() >= VL53L0X_OPPENING) set_led(LED1, 1);
		if(get_calibrated_prox(IR3) <= NOISE_IR) set_led(LED3, 1);
		set_led(LED5, 1);
		if(get_calibrated_prox(IR6) <= NOISE_IR) set_led(LED7, 1);

		middle_reached = false;
	 }
}

//Developpement

uint32_t absolute_value_int32(int32_t value){
	if(value < 0) value = -value;
	return value;
}

void PID_tuning(void){
	static int state = 0;
	int max_state = 3;

	if(button_get_state() == 1)
	{
		if(state <= max_state) state++;
		else state = 0;
	}

	switch(state)
	{
	case 0: set_rgb_led(LED2, 0,0,0); break;
	case 1: set_rgb_led(LED2, 100,0,0); Kp_tun += incr * infinity_selector();
	chprintf((BaseSequentialStream *)&SD3, "KP = %.2f \n\n\r", Kp_tun);break;
	case 2: set_rgb_led(LED2, 0,100,0); Kd_tun += incr * infinity_selector();
	chprintf((BaseSequentialStream *)&SD3, "KD = %.2f \n\n\r", Kd_tun);break;
	case 3: break;
	}

}

int infinity_selector(void){
	static int last_pos = 0;
	int pos = get_selector();

	if(pos == last_pos) return(0);
	if(pos == 0 && last_pos == 16){
		last_pos = pos;
		return(1);
	}
	if(pos == 16 && last_pos == 0){
		last_pos = pos;
		return(-1);
	}
	if(pos > last_pos){
		last_pos = pos;
		return(1);
	}
	if(pos < last_pos){
		last_pos = pos;
		return(-1);
	}

}
