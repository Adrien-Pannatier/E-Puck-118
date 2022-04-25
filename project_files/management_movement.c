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
#include "handle_fire.h"

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

//Angle related
#define MIN_ANGLE						0
#define MAX_ANGLE						360
#define STEP_DEG						(1316.0/360.0)									//[step]
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

//Static variables accessible from outside
static uint8_t movement_state = STOP;
static int16_t orientation = 0;
static int16_t orientation_before_check = 0;
static bool fire_detected = false;
static bool opening_right = false, opening_left = false, opening_front = false;

//Store history of navigation
static int16_t buffer_navigation_history[HISTORY_SIZE];
static int8_t ptr_buffer_nav = 0;

//DEV
//oooooooooooooooooooooooooooooooooooooooooooooooooo

static float Kd_tun = 1.65;
static float Kp_tun = 0.06;
const float incr = 0.05;

//ooooooooooooooooooooooooooooooooooooooooooooooooooo

//Functions
bool opening_found(void);
bool dead_end_found(void);
bool corridor_found(void);
void stop_movement(void);
void followind_corridor(void);
void moving_in_intersection(void);
void update_orientation(int rotation_angle);
void rotate(int rotation_angle);
void analysing_intersection(void);
void join_corridor(void);

//Thd gestion movement of the robot

static THD_WORKING_AREA(waThdMovement, 512);
static THD_FUNCTION(Movement, arg) {

    chRegSetThreadName(__FUNCTION__);

    //Thread motors init
    motors_init();
//    systime_t time;

    while(1){

//    	time = chVTGetSystemTime();

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


    	case MOVING: 				followind_corridor(); break;


    	case REACHING_INTERSECTION: moving_in_intersection(); break;


    	case ROTATING: 				analysing_intersection(); break;


    	case LEAVING_INTERSECTION: 	join_corridor(); break;

    	case SEARCHING_FIRE:		searching_for_fire(); break;

    	case FIRE_FIGHTING: 		fighting_fire(); break;

    	default: movement_state = STOP; break;
    	}
//
//    	if(get_selector() == 0) chprintf((BaseSequentialStream *)&SD3, "Thd time = %d\n\n\r", chVTGetSystemTime()-time);

    	chThdSleepMilliseconds(50);
    }
}

//DEV
//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo

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

//oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo


//Access functions

bool get_fire_detected(void){
	return fire_detected;
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

bool opening_found(void){

	//if opening detected
	if(get_calibrated_prox(IR2) < NOISE_IR || get_calibrated_prox(IR7) < NOISE_IR)
	{
		return true;
	}
	return false;
}

bool dead_end_found(void){

	static uint8_t certainty_counter = 0;

	//if front is obstructed return complete

	if(VL53L0X_get_dist_mm() <= VL53L0X_OBSTRUCTED)
	{
		if(certainty_counter >= CERTAINTY)
		{
			certainty_counter = 0;
			return true;
		}
		//counter to avoid noise
		certainty_counter++;
	}
	else if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING) certainty_counter = 0;

	return false;
}


//Use a PD regulator to follow a corridor and check for opening or dead end

void followind_corridor(void){

	//Variables PD
	int32_t error = 0;
	int32_t past_error = 0;
	int16_t correction = 0;

	//Reset counter left motor for mapping
	left_motor_set_pos(NULL_POS);

	while(1){

		//PD
		error = get_calibrated_prox(IR3) - get_calibrated_prox(IR6);

		//Calculation of trajectory correction
		correction = Kp_tun * error + Kd_tun * (error - past_error);
		past_error = error;

		//Smoother
		if(correction > ROTATIONAL_SPEED) correction = ROTATIONAL_SPEED;
		else if(correction < -ROTATIONAL_SPEED) correction = -ROTATIONAL_SPEED;

		//set motor speed
		right_motor_set_speed(SPEED_STEP + correction);
		left_motor_set_speed(SPEED_STEP - correction);

		//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_corridor();
			left_motor_set_pos(NULL_POS);
		}

//		//Save history of movements for trajectory correction
//		buffer_navigation_history[ptr_buffer_nav] = correction;
//		ptr_buffer_nav++;
//		if(ptr_buffer_nav == HISTORY_SIZE) ptr_buffer_nav = 0;

		//Check for dead end or opening
		if(opening_found()){
			movement_state = REACHING_INTERSECTION;
			break;
		}
		if(dead_end_found()){
			movement_state = ROTATING;
			break;
		}

		chThdSleepMilliseconds(50);
	}
}

bool corridor_found(void){

	static uint8_t certainty_counter = 0;

	if(get_calibrated_prox(IR3) >= NOISE_IR && get_calibrated_prox(IR6) >= NOISE_IR)
		{
			if(certainty_counter >= CERTAINTY)
			{
				certainty_counter = 0;
				return true;
			}
			else certainty_counter++;
		}
	else if(get_calibrated_prox(IR3) <= NOISE_IR || get_calibrated_prox(IR6) <= NOISE_IR) certainty_counter = 0;

	return false;
}

//bool trajectory_correction(void){
//
//	static uint8_t counter = 0;
//
//	//Rewind back some movements after a wrong correction
//	if(ptr_buffer_nav == 0) ptr_buffer_nav = HISTORY_SIZE;
//	ptr_buffer_nav--;
//
//	right_motor_set_speed(SPEED_STEP - buffer_navigation_history[ptr_buffer_nav]);
//	left_motor_set_speed(SPEED_STEP + buffer_navigation_history[ptr_buffer_nav]);
//
//	counter++;
//	if(counter == HISTORY_SIZE){
//		counter = 0;
//		return COMPLETE;
//	}
//
//	return NOT_COMPLETE;
//}

void moving_in_intersection(void){

	//Reset the right motor counter for position
	right_motor_set_pos(0);

	//Reset counter left motor for mapping
	left_motor_set_pos(NULL_POS);

	//Correction of trajectory
//	trajectory_correction();

	//Start moving forward in the crossing
	right_motor_set_speed(SPEED_STEP);
	left_motor_set_speed(SPEED_STEP);

	while(1){

		//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_moving_in_intersection();
			left_motor_set_pos(NULL_POS);
		}

		//Check if a corridor has been found again
		if(corridor_found()){
			movement_state = MOVING;
			break;
		}

		//Stop when the middle is reached
		if(right_motor_get_pos() >= STEP_TO_REACH_THE_MIDDLE)
		{
			stop_movement();
			movement_state = ROTATING;
			break;
		}

		chThdSleepMilliseconds(20);
	}
}

void update_orientation(int rotation_angle){

	orientation = orientation + rotation_angle;

	//Modulo 2pi
	if(orientation >= MAX_ANGLE) orientation -= MAX_ANGLE;
	else if(orientation < MIN_ANGLE) orientation += MAX_ANGLE;

	//Send update to buffer
	send_orientation(orientation);

}

void rotate(int rotation_angle){

	int32_t right_motor_pos;

	//Reset the counter
	right_motor_set_pos(0);

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

	while(1){

		//stop when desired angle is reached
		right_motor_pos = right_motor_get_pos();
		if(absolute_value_int32(right_motor_pos) >= absolute_value_int32(rotation_angle * STEP_DEG))
		{
			stop_movement();
			break;
		}

		chThdSleepMilliseconds(20);
	}

	update_orientation(rotation_angle);
}

void analysing_intersection(void){

	//Check for opening
	if(VL53L0X_get_dist_mm() >= VL53L0X_OPENING){
		opening_front = true;
	//	set_led(LED1, 1);
	}
	if(get_calibrated_prox(IR3) <= NOISE_IR){
		opening_right = true;
	//	set_led(LED3, 1);
	}
	if(get_calibrated_prox(IR6) <= NOISE_IR){
		opening_left = true;
	//	set_led(LED7, 1);
	}

	//Send for mapping
	send_crossing(opening_right, opening_front, opening_left);

//	if(opening_right){
//		rotate(RIGHT_90);
//	}
//	else if(opening_front){
//	}
//	else if(opening_left){
//		rotate(LEFT_90);
//	}
	//Changing movement state
	orientation_before_check = orientation;
	movement_state = SEARCHING_FIRE;
	//reset opening bool
//	opening_front = false;
//	opening_left = false;
//	opening_right = false;
}

void join_corridor(void){

	//go forward until a corridor is reached
	right_motor_set_speed(SPEED_STEP);
	left_motor_set_speed(SPEED_STEP);

	//Reset counter for mapping
	left_motor_set_pos(NULL_POS);

	while(1){

		//Send movement for transmission every 1cm
		if(left_motor_get_pos() >= (NSTEP_ONE_TURN / WHEEL_PERIMETER)){
			send_moving_in_intersection();
			left_motor_set_pos(NULL_POS);
		}

		//Check for corridor
		if(corridor_found()){
			opening_front = false;
			opening_left = false;
			opening_right = false;

			movement_state = MOVING;

			break;
		}

		//Check for dead end
		if(dead_end_found()){

			movement_state = ROTATING;

			break;
		}
	}
//			clear_leds();
}
void searching_for_fire(void){
	//checking front
	if(opening_front){
		if(check_for_fire()){
			//fire procedure
			opening_front = false;
			movement_state = FIRE_FIGHTING;
			return;
		}
	}
	if(opening_left){
		rotate(LEFT_90);
		if(check_for_fire()){
			//fire procedure
			opening_left = false;
			movement_state = FIRE_FIGHTING;
			return;
		}
		else if(opening_right){
			rotate(RIGHT_180);
			if(check_for_fire()){
				//fire procedure
				opening_right = false;
				movement_state = FIRE_FIGHTING;
				return;
			}
		}
	}
	if(opening_right){
		rotate(RIGHT_90);
		if(check_for_fire()){
			//fire procedure
			opening_right = false;
			movement_state = FIRE_FIGHTING;
			return;
		}
	}
	movement_state = LEAVING_INTERSECTION;
}

void fighting_fire(void){
	deploy_antifire_measures();
	chThdSleepMilliseconds(1000);
	stop_antifire_measures();
	if(check_for_fire() == false){
		movement_state = SEARCHING_FIRE;
		//remettre l'orientation de base
	}
}

void management_movement_start(void){
	   chThdCreateStatic(waThdMovement, sizeof(waThdMovement), NORMALPRIO, Movement, NULL);
}

