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


//Moving state
#define STOP							0
#define MOVING							1
#define REACHING_INTERSECTION			2
#define ROTATING						3
#define LEAVING_INTERSECTION			4
#define FIRE_FIGHTING					5

//Moving parameters
#define CERTAINTY						10												//minimum occurrence of a measure to have a good confidence
#define R_EPUCK							3.5												//[cm]
#define MAZE_WIDTH						10												//[cm]
#define NSTEP_ONE_TURN   				1000 											//number of step for 1 turn of the motor
#define WHEEL_PERIMETER     			13 												//[cm]
#define SPEED							6												//[cm/s]
#define SPEED_STEP						(SPEED*NSTEP_ONE_TURN/WHEEL_PERIMETER)			//[step/s]
#define ROTATIONAL_SPEED				280
#define STEP_TO_REACH_THE_MIDDLE		320 //(MAZE_WIDTH/2*NSTEP_ONE_TURN/WHEEL_PERIMETER) 	//[step]
#define SPEED_NUL						0
#define RIGHT_360						1316											//[step]
#define RIGHT_180						658												//[step]
#define RIGHT_90						329												//[step]
#define LEFT_360						(-RIGHT_360)									//[step]
#define LEFT_180						(-RIGHT_180)									//[step]
#define LEFT_90							(-RIGHT_90)										//[step]


//PD parameters
#define ANTI_WINDUP						5000
#define KP								1.65
#define KD								0.06


//a changer
#define VL53L0X_OPENING					100												//distance considered for opening [mm]


static uint8_t movement_state = STOP;
static bool fire_detected = false;
static bool opening_right = false, opening_left = false, opening_front = false;

//provisoir pour tunning
static float Kd_tun = 1.65;
static float Kp_tun = 0.06;
const float incr = 0.05;

//A changer si possible
const bool COMPLETE = true;																//state returned by a task when it's completed
const bool NOT_COMPLETE = false;														//state returned by a task while it's not completed


//DEV
int32_t absolute_value_int32(int32_t value){
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
	case 0: break;
	case 1: set_led(LED1, 1); Kp_tun += incr * infinity_selector();
	chprintf((BaseSequentialStream *)&SD3, "KP = %.2f \n\n\r", Kp_tun);break;
	case 2: set_led(LED3, 1); Kd_tun += incr * infinity_selector();
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
	return(0);
}
int16_t absolute_value_int16(int16_t value){
	if(value < 0) value = -value;
	return value;
}



bool get_fire_detected(void){
	return(fire_detected);
}


//Movement related functions

void stop_movement(void){
	right_motor_set_speed(SPEED_NUL);
	left_motor_set_speed(SPEED_NUL);
}

void movement_regulation(void)
{
	//Variables PD
	int32_t error = 0;
	static int32_t past_error = 0;
	int16_t correction = 0;

	//PD
	error = get_calibrated_prox(IR3) - get_calibrated_prox(IR6); // get_intensity(IR3) - get_intensity(IR6);//)* 5.0 + get_intensity(IR2) - get_intensity(IR5) + get_intensity(IR4) - get_intensity(IR7);
	//error = get_distance_IR_mm(IR6) - get_distance_IR_mm(IR3);

	//Calculation of trajectory correction
	correction = Kp_tun * error + Kd_tun * (error - past_error);
	past_error = error;

	//set motor speed
	right_motor_set_speed(SPEED_STEP + correction);
	left_motor_set_speed(SPEED_STEP - correction);

}


bool check_for_openings(void){

	//if opening detected
//	if(get_distance_IR_mm(IR3) >= MAX_RANGE_IR || get_distance_IR_mm(IR6) >= MAX_RANGE_IR) //a voir la meilleure solution
	if(get_calibrated_prox(IR3) <= NOISE_IR || get_calibrated_prox(IR6) <= NOISE_IR)
	{
		return COMPLETE;
	}
	return NOT_COMPLETE;
}

bool check_for_corridor(void){

	static uint8_t certainty_counter = 0;

	if(get_calibrated_prox(IR3) >= NOISE_IR && get_calibrated_prox(IR6) >= NOISE_IR)
		{
			if(certainty_counter >= CERTAINTY)
				{
					certainty_counter = 0;
					return COMPLETE;
				}
			//counter to avoid noise
			certainty_counter++;
		}
	return NOT_COMPLETE;
}

bool moving_in_intersection(void){

	static bool init_counter = false;


	//init the counter only one time and start moving forward
	if(!init_counter)
	{
		right_motor_set_pos(0);
		init_counter = true;

		right_motor_set_speed(SPEED_STEP);
		left_motor_set_speed(SPEED_STEP);
	}

	//stop when the middle is reached
	if(right_motor_get_pos() >= STEP_TO_REACH_THE_MIDDLE)
	{
	    stop_movement();
	    init_counter = false;
	    movement_state = ROTATING;

	    //store openings
		if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING) opening_front = true;
		if(get_calibrated_prox(IR3) <= NOISE_IR) opening_right = true;
		if(get_calibrated_prox(IR6) <= NOISE_IR) opening_left = true;

		return COMPLETE;
	}
	return NOT_COMPLETE;
}

bool rotate(int rotation_angle){
	static bool init_counter = false;
	int32_t right_motor_pos;

	//init the counter only one time and start rotating
	if(!init_counter)
	{
		right_motor_set_pos(0);
		init_counter = true;

		//adjust rotation direction
		if(rotation_angle > 0)
		{
			right_motor_set_speed(-ROTATIONAL_SPEED);
			left_motor_set_speed(ROTATIONAL_SPEED);
		}
		else
		{
			right_motor_set_speed(ROTATIONAL_SPEED);
			left_motor_set_speed(-ROTATIONAL_SPEED);
		}
	}

	//stop when desired angle is reached
	right_motor_pos = right_motor_get_pos();
	if(absolute_value_int32(right_motor_pos) >= absolute_value_int32(rotation_angle))
	{
		stop_movement();
		init_counter = false;

		return COMPLETE;
	}
	return NOT_COMPLETE;
}

bool choose_next_path(){

	//Follow the right wall, return complete when the rotation is done
	if(opening_right)
	{
		if(rotate(RIGHT_90) == COMPLETE) return COMPLETE;
		else return NOT_COMPLETE;
	}
	if(opening_front)
	{
		return COMPLETE;
	}
	if(opening_left)
	{
		if(rotate(LEFT_90) == COMPLETE) return COMPLETE;
		else return NOT_COMPLETE;
	}
	else
	{
		if(rotate(RIGHT_180) == COMPLETE) return COMPLETE;
		else return NOT_COMPLETE;
	}
}

static THD_WORKING_AREA(waThdMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);

    //Thread motors init
    motors_init();

    while(1){

    	//PID_tuning();

    	//restart movement
    	if(button_get_state() == 1)
    	{
    		chThdSleepSeconds(1);
    		clear_leds();
    		movement_state = MOVING;
    	}

    	switch(movement_state){

    	//No movement
    	case STOP: stop_movement(); break;

    	//Following a corridor
    	case MOVING: movement_regulation();
    		if(check_for_openings() == COMPLETE){
    			movement_state = REACHING_INTERSECTION;
    			set_led(LED1, 1);
    		}
    		break;

    	//Reaching the middle of an intersection
    	case REACHING_INTERSECTION: if(moving_in_intersection() == COMPLETE) movement_state = ROTATING;
    		else if(check_for_corridor() == COMPLETE) movement_state = MOVING; break;

    	//Choosing the next path and rotating in the intersection
    	case ROTATING: if(choose_next_path() == COMPLETE){
    			movement_state = STOP;
    			opening_front = false;
    			opening_left = false;
    			opening_right = false;
    			set_led(LED3, 1);
    		}
    		break;

    	//Regaining the corridor
    	case LEAVING_INTERSECTION: break;

    	//Taking actions against fire
    	case FIRE_FIGHTING: break;

    	default: movement_state = STOP; break;
    	}

    	chThdSleepMilliseconds(50);

    }
}

void management_movement_start(void){
	   chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
//	   chThdCreateStatic(waThdDisasterAreaDisplacement, sizeof(waThdDisasterAreaDisplacement), NORMALPRIO, DisasterAreaDisplacement, NULL);
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
//			right_motor_set_speed(SPEED_STEP + correction);
//			left_motor_set_speed(SPEED_STEP - correction);
//
//			//check for openings
//			check_for_openings();
//    	}
//    	chThdSleepMilliseconds(50);
//
//    }
//}
