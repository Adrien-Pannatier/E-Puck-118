/*
 * light_gestion.c
 *
 *  Created on: 12 avr. 2022
 *      Author: APrap
 *
 *  Thread declaration and defines for LED responses depending on
 *  the state of the robot
 */
#include "light_gestion.h"
#include "ch.h"
#include "hal.h"
#include "leds.h"
#include "management_movement.h"

static THD_WORKING_AREA(waThdLED, 128);
static THD_FUNCTION(ThdLED, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while (1) {

    	if(get_movement_state() == STOP)
    	{
    		//LEDs OFF

    		clear_leds();
    		chThdSleepMilliseconds(300);
    	}
    	else if(get_movement_state() == FIRE_FIGHTING)
       	{
    		//Alarm rotation of red leds

    		clear_leds();
    		set_led(LED1, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		//set_rgb_led(LED2, RGB_RED);
    		set_led(LED1, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED3, LED_ON);
    		//set_rgb_led(LED2, RGB_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		//set_rgb_led(LED4, RGB_RED);
    		set_led(LED3, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED5, LED_ON);
    		//set_rgb_led(LED4, RGB_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		//set_rgb_led(LED6, RGB_RED);
    		set_led(LED5, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED7, LED_ON);
    		//set_rgb_led(LED6, RGB_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		//set_rgb_led(LED8, RGB_RED);
    		set_led(LED7, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);


       	}
    	else if(get_movement_state() == SEARCHING_FIRE)
       	{
    		//Alarm rotation of red leds

    		clear_leds();

    		set_led(LED1, LED_ON);
    		set_led(LED5, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED1, LED_OFF);
    		set_led(LED5, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
       		set_led(LED3, LED_ON);
    		set_led(LED7, LED_ON);
    		chThdSleepMilliseconds(ALAMRM_SPEED);
    		set_led(LED3, LED_OFF);
    		set_led(LED7, LED_OFF);
    		chThdSleepMilliseconds(ALAMRM_SPEED);



       	}
    	else if(get_movement_state() == END_OF_MAZE){
    		set_body_led(1);
    	    chThdSleepMilliseconds(ALAMRM_SPEED);
    	    set_body_led(0);
    	    chThdSleepMilliseconds(ALAMRM_SPEED);
    	}
       	else //if(get_movement_state() == other)
       	{
       		//Siren fire-fighter truck

//   			set_rgb_led(LED2, RGB_BLUE);
//   			set_rgb_led(LED4, RGB_RED);
//   			set_rgb_led(LED6, RGB_BLUE);
//   			set_rgb_led(LED8, RGB_RED);

//   			chThdSleepMilliseconds(300);
//
//   			set_rgb_led(LED2, RGB_RED);
//   			set_rgb_led(LED4, RGB_BLUE);
//   			set_rgb_led(LED6, RGB_RED);
//   			set_rgb_led(LED8, RGB_BLUE);
//
//   			chThdSleepMilliseconds(300);

     	}
    }
}

void LED_start(void){
	chThdCreateStatic(waThdLED, sizeof(waThdLED), NORMALPRIO -1, ThdLED, NULL);
}
