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
#include <usbcfg.h>
#include "chprintf.h"

//Sensor value (0-4000)
static uint16_t tab_prox[PROXIMITY_NB_CHANNELS] = {0};

static THD_WORKING_AREA(waThdManagementProximity, 512);
static THD_FUNCTION(ThdManagementProximity, arg) {

    chRegSetThreadName("ManagementProximityThd");
    (void)arg;

    //Sensor clock init

    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN;

    //Start proximity sensors
    proximity_start();
    calibrate_ir();

    //Realterm
    usb_start();
    serial_start();

    while(1){

    	for(int i = 0; i < PROXIMITY_NB_CHANNELS; i++){
    		tab_prox[i] = get_calibrated_prox(i);
    	}


//    	chprintf((BaseSequentialStream *)&SD3, "IR1 = %u IR2 = %u IR3 = %u IR4 = %u IR5 = %u IR6 = %u IR7 = %u IR8 = %u \n\n\r",
//    			tab_prox[0], tab_prox[1], tab_prox[2], tab_prox[3], tab_prox[4], tab_prox[5], tab_prox[6], tab_prox[7]);

    	chThdSleepMilliseconds(500);

    }
}

void management_proximity_start(void){
	chThdCreateStatic(waThdManagementProximity, sizeof(waThdManagementProximity), NORMALPRIO, ThdManagementProximity, NULL);
}

uint16_t get_distance(uint8_t IR_number){

	//Check if IR_number is a valid number, return the distance in intensity (0-4000)

	if(IR_number <= 8) return(tab_prox[IR_number]);
	else return(0);
}

// COM

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

