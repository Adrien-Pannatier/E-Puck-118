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
#include "management_transmissions.h"

//provisoir pour debug
#include "leds.h"
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"
#include "chprintf.h"
#include "button.h"
#include "selector.h"


//Moving parameters
#define CERTAINTY						3												//minimum occurrence of a measure to have a good confidence
#define NULL_POS						0												//Reset position zero compteur moteur
#define R_EPUCK							3.5												//[cm]
#define MAZE_WIDTH						10.0											//[cm]
#define NSTEP_ONE_TURN   				1000 											//number of step for 1 turn of the motor
#define WHEEL_PERIMETER     			13.0 												//[cm]
#define SPEED							6.0												//[cm/s]
#define SPEED_STEP						(SPEED*NSTEP_ONE_TURN/WHEEL_PERIMETER)			//[step/s] ()
#define ROTATIONAL_SPEED				280
#define STEP_TO_REACH_THE_MIDDLE		320											 	//[step]
#define SPEED_NUL						0
#define HISTORY_SIZE					40												//Size of navigation history buffer (store history of movements)
#define STEP_DEG						(1316.0/360.0)										//[step]
#define RIGHT_360						360
#define RIGHT_180						180												//[step]
#define RIGHT_90						90												//[step]
#define LEFT_360						-360											//[step]
#define LEFT_180						-180											//[step]
#define LEFT_90							-90												//[step]


//PD parameters
#define ANTI_WINDUP						5000
#define KP								1.65
#define KD								0.06


//a changer
#define VL53L0X_OPENING					100												//distance considered for opening [mm]
#define VL53L0X_OBSTRUCTED				50												//distance considered for obstructed [mm]

static uint8_t movement_state = STOP;
static int16_t orientation = 0;
static bool fire_detected = false;
static bool opening_right = false, opening_left = false, opening_front = false;
static bool init_counter_reaching_intersection = false;
static bool correction_trajectory_complete = false;
static int16_t buffer_navigation_history[HISTORY_SIZE];
static int8_t ptr_buffer_nav = 0;

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
	case 3: set_led(LED5, 1);
	break;
	}

}



bool get_fire_detected(void){
	return(fire_detected);
}

uint8_t get_movement_state(void){
	return movement_state;
}

uint8_t get_orientation(void){
	return orientation;
}


//Movement related functions

void stop_movement(void){
	right_motor_set_speed(SPEED_NUL);
	left_motor_set_speed(SPEED_NUL);
}

void reset_moving_in_intersection(void){
	init_counter_reaching_intersection = false;
	correction_trajectory_complete = NOT_COMPLETE;
}

void movement_regulation(void)
{
	//Variables PD
	int32_t error = 0;
	static int32_t past_error = 0;
	int16_t correction = 0;

	//PD
	error = get_calibrated_prox(IR3) - get_calibrated_prox(IR6);

	//Calculation of trajectory correction
	correction = Kp_tun * error + Kd_tun * (error - past_error);
	past_error = error;

	//Smoother
	if(get_selector() == 1){
			if(correction > ROTATIONAL_SPEED) correction = ROTATIONAL_SPEED;
			else if(correction < -ROTATIONAL_SPEED) correction = -ROTATIONAL_SPEED;
	}


	//set motor speed
	right_motor_set_speed(SPEED_STEP + correction);
	left_motor_set_speed(SPEED_STEP - correction);

	//Send movement for transmission every 1cm
	if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
		send_corridor();
		left_motor_set_pos(NULL_POS);
	}

	//Save history of movements
	buffer_navigation_history[ptr_buffer_nav] = correction;
	ptr_buffer_nav++;
	if(ptr_buffer_nav == HISTORY_SIZE) ptr_buffer_nav = 0;

}


bool check_for_openings(void){

	//if opening detected
	if(get_calibrated_prox(IR2) < NOISE_IR || get_calibrated_prox(IR7) < NOISE_IR)
	{
		return COMPLETE;

		set_led(LED1, 1);

		//Init counter left motor for mapping
		left_motor_set_pos(NULL_POS);
	}
	return NOT_COMPLETE;
}

bool check_for_dead_end(void){

	static uint8_t certainty_counter = 0;

	//if front is obstructed return complete

	if(VL53L0X_get_dist_mm() <= VL53L0X_OBSTRUCTED)
	{
		if(certainty_counter >= CERTAINTY)
		{
			certainty_counter = 0;
			return COMPLETE;
		}
		//counter to avoid noise
		certainty_counter++;
	}
	else if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING) certainty_counter = 0;

	return NOT_COMPLETE;
}

bool check_for_corridor(void){

	static uint8_t certainty_counter = 0;

	if(get_calibrated_prox(IR3) >= NOISE_IR && get_calibrated_prox(IR6) >= NOISE_IR)
		{
			if(certainty_counter >= CERTAINTY)
				{
					certainty_counter = 0;

					//Reset counter left motor for mapping
					left_motor_set_pos(NULL_POS);

					//Reset parameters for intersection when avoided and not completed
					reset_moving_in_intersection();

					return COMPLETE;
				}
			//counter to avoid noise
			certainty_counter++;
		}
	else if(get_calibrated_prox(IR3) <= NOISE_IR || get_calibrated_prox(IR6) <= NOISE_IR) certainty_counter = 0;

	return NOT_COMPLETE;
}

bool trajectory_correction(void){

	static uint8_t counter = 0;

	//Rewind back some movements after a wrong correction
	if(ptr_buffer_nav == 0) ptr_buffer_nav = HISTORY_SIZE;
	ptr_buffer_nav--;

	right_motor_set_speed(SPEED_STEP - buffer_navigation_history[ptr_buffer_nav]);
	left_motor_set_speed(SPEED_STEP + buffer_navigation_history[ptr_buffer_nav]);

	counter++;
	if(counter == HISTORY_SIZE){
		counter = 0;
		return COMPLETE;
	}

	return NOT_COMPLETE;
}

bool moving_in_intersection(void){

	//init the counter only one time
	if(!init_counter_reaching_intersection)
	{
		right_motor_set_pos(0);
		init_counter_reaching_intersection = true;
	}

	//Init the correction of trajectory
	if(!correction_trajectory_complete)
	{
			if(trajectory_correction() == COMPLETE) correction_trajectory_complete = COMPLETE;
	}
	else
	{
		right_motor_set_speed(SPEED_STEP);
		left_motor_set_speed(SPEED_STEP);
	}

	//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_moving_in_intersection();
			left_motor_set_pos(NULL_POS);
		}

	//stop when the middle is reached
	if(right_motor_get_pos() >= STEP_TO_REACH_THE_MIDDLE)
	{
	    stop_movement();
	    reset_moving_in_intersection();

		return COMPLETE;
	}
	return NOT_COMPLETE;
}

//MAGIC NUMBERS

void update_orientation(int rotation_angle){

	orientation = orientation + rotation_angle;

	//Modulo 360
	if(orientation >= 360) orientation = orientation - 360;
	else if(orientation < 0) orientation = orientation + 360;

	//Send update to buffer
	send_orientation(orientation);

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
	if(absolute_value_int32(right_motor_pos) >= absolute_value_int32(rotation_angle * STEP_DEG))
	{
		stop_movement();
		init_counter = false;

		return COMPLETE;
	}
	return NOT_COMPLETE;
}

bool choose_next_path(void){

	static bool check_openings = NOT_COMPLETE;

	//store openings
	if(check_openings == NOT_COMPLETE){

		if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING){
			opening_front = true;
//			set_led(LED1, 1);
		}
		if(get_calibrated_prox(IR3) <= NOISE_IR){
			opening_right = true;
//			set_led(LED3, 1);
		}
		if(get_calibrated_prox(IR6) <= NOISE_IR){
			opening_left = true;
//			set_led(LED7, 1);
		}

		//Send crossing for mapping
		send_crossing(opening_right, opening_front, opening_left);

		check_openings = COMPLETE;
	}



	//Follow the right wall, return complete when the rotation is done
	if(opening_right)
	{
		if(rotate(RIGHT_90) == COMPLETE){
			check_openings = NOT_COMPLETE;
			return COMPLETE;
		}
		else return NOT_COMPLETE;
	}
	if(opening_front)
	{
		check_openings = NOT_COMPLETE;
		return COMPLETE;
	}
	if(opening_left)
	{
		if(rotate(LEFT_90) == COMPLETE){
			check_openings = NOT_COMPLETE;
			return COMPLETE;
		}
		else return NOT_COMPLETE;
	}
	else
	{
		if(rotate(RIGHT_180) == COMPLETE){
			check_openings = NOT_COMPLETE;
			return COMPLETE;
		}
		else return NOT_COMPLETE;
	}
}

bool join_corridor(void){

	//go forward until a corridor is reached
	right_motor_set_speed(SPEED_STEP);
	left_motor_set_speed(SPEED_STEP);

	//Send movement for transmission every 1cm
	if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
		send_moving_in_intersection();
		left_motor_set_pos(NULL_POS);
	}

	if(check_for_corridor() == COMPLETE){

		//reset opening bool and speed init
		opening_front = false;
		opening_left = false;
		opening_right = false;

		clear_leds();

		return COMPLETE;
	}

	return NOT_COMPLETE;
}

static THD_WORKING_AREA(waThdMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);

    //Thread motors init
    motors_init();
    systime_t time;

    while(1){

    	time = chVTGetSystemTime();

    	//chprintf((BaseSequentialStream *)&SD3, "Thd time = %d\n", thd_time);

    	//PID_tuning();

    	//Selector control
    	if(get_selector() == 0) movement_state = STOP;
    	else if(movement_state == STOP){
    		chThdSleepMilliseconds(500);
    		movement_state = LEAVING_INTERSECTION;
    	}


    	switch(movement_state){


       	case STOP: 					stop_movement(); break;


    	case MOVING: 				movement_regulation();
    								if(check_for_openings() == COMPLETE) movement_state = REACHING_INTERSECTION;
    								if(check_for_dead_end() == COMPLETE) movement_state = ROTATING; break;


    	case REACHING_INTERSECTION: if(moving_in_intersection() == COMPLETE) movement_state = ROTATING;
    								else if(check_for_corridor() == COMPLETE) movement_state = MOVING; break;


    	case ROTATING: 				if(choose_next_path() == COMPLETE) movement_state = LEAVING_INTERSECTION;break;


    	case LEAVING_INTERSECTION: if(join_corridor() == COMPLETE) movement_state = MOVING;
    							   if(check_for_dead_end() == COMPLETE) movement_state = ROTATING; break;

    	case FIRE_FIGHTING:		   //if (check_for_fire() == true)
    							   break; //do wiggle

    	default: movement_state = STOP; break;
    	}

//    	chThdSleepMilliseconds(100);
//
//    	if(get_selector() == 0) chprintf((BaseSequentialStream *)&SD3, "Thd time = %d\n\n\r", chVTGetSystemTime()-time);

    	chThdSleepUntilWindowed(time, time + MS2ST(50));

    }
}

void management_movement_start(void){
	 chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
}

