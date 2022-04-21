/*
 * Proximity.c
 *
 *  Created on: 8 avr. 2022
 *      Author: APrap
 */

#include <management_proximity.h>
#include <msgbus/messagebus.h>
#include "ch.h"
#include "hal.h"
#include "leds.h"
#include <stm32f4xx.h>
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"
#include "math.h"
#include "chprintf.h"

//interpolation for linearisation IR value
#define MUL_VAL			1324.6
#define POWER			-0.79

//correspondance table
static uint8_t table_lin_IR[600] = {200};

//Sensor value (0-4000)
static uint16_t tab_prox[PROXIMITY_NB_CHANNELS];
static uint16_t forward_dist = 0;

static THD_WORKING_AREA(waThdManagementProximity, 1024);
static THD_FUNCTION(ThdManagementProximity, arg) {

    chRegSetThreadName("ManagementProximityThd");
    (void)arg;

    static bool calibrated = false;

    //Sensor clock init

    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN;

    //Start proximity sensors
    proximity_start();
    calibrate_ir();
    VL53L0X_start();

    //file table lin IR sensors
    for(int i = 0; i < 30; i++)
    {
    	table_lin_IR[i] = MAX_RANGE_IR;
    }
    for(int i = 30; i < 600; i++)
    {
    	table_lin_IR[i] = (uint8_t) (MUL_VAL*powf((i), POWER));
    }

    while(1){

    	for(int i = 0; i < PROXIMITY_NB_CHANNELS; i++){
    		tab_prox[i] = get_calibrated_prox(i);
    	}
    	forward_dist = VL53L0X_get_dist_mm();


//    	chprintf((BaseSequentialStream *)&SD3, "Distance IR3 = %u \n\n\r", get_distance_IR_mm(IR3));
//    	chprintf((BaseSequentialStream *)&SD3, "Intensity IR3 = %u \n\n\r", get_calibrated_prox(IR3));

//    	chprintf((BaseSequentialStream *)&SD3, "Distance = %u \n\n\r", forward_dist);
//    	chprintf((BaseSequentialStream *)&SD3, "IR1 = %u IR2 = %u IR3 = %u IR4 = %u IR5 = %u IR6 = %u IR7 = %u IR8 = %u \n\n\r",
//    			tab_prox[0], tab_prox[1], tab_prox[2], tab_prox[3], tab_prox[4], tab_prox[5], tab_prox[6], tab_prox[7]);

//    	tab_prox[IR3] = get_intensity(IR3);
//    	tab_prox[IR4] = get_calibrated_prox(IR3);
//    	chprintf((BaseSequentialStream *)&SD3, "IR3 normalised = %u, IR3 raw = %u \n\n\r", tab_prox[IR3], tab_prox[IR4]);

    	chThdSleepMilliseconds(50);

    }
}

void management_proximity_start(void){
	chThdCreateStatic(waThdManagementProximity, sizeof(waThdManagementProximity), NORMALPRIO, ThdManagementProximity, NULL);
}

uint8_t get_distance_IR_mm(uint8_t IR_number){

	uint16_t intensity = get_calibrated_prox(IR_number);
	if(intensity > 599) intensity = 599;

	//Check if IR_number is a valid number, return the distance in mm from the transformation table
	if(IR_number <= 8)
		{
		if(intensity > 599) intensity = 599;
		return(table_lin_IR[intensity]);
		}
	else return(0);
}


