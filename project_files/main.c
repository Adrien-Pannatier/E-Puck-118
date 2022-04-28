#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "spi_comm.h"
#include "memory_protection.h"
#include <main.h>
#include <motors.h>
#include <audio/microphone.h>
#include <leds.h>

#include <msgbus/messagebus.h>
#include <audio/audio_thread.h>
#include <audio/play_melody.h>
#include <sensors/proximity.h>
#include <added_melodies.h>
#include "chmtx.h"
#include "light_gestion.h"
#include "audio/audio_thread.h"
#include "management_proximity.h"
#include "management_movement.h"
#include "management_transmissions.h"
#include "usbcfg.h"
#include "spi_comm.h"

#include <process_image.h>
#include <camera/po8030.h>

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

//DEV
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


int main(void)
{
	//Systeme
    halInit();
    chSysInit();
    mpu_init();

    //LED
    spi3_slave_lld_init();
    spi_comm_start();
    LED_start();

    dac_start();
    playMelodyStart();

    //Camera
    dcmi_start();
    po8030_start();
    process_image_start();

    while(get_selector() != 0){
    	//Wait for 0
		set_rgb_led(LED2, RGB_RED);
		set_rgb_led(LED4, RGB_RED);
		set_rgb_led(LED6, RGB_RED);
		set_rgb_led(LED8, RGB_RED);
    }
	set_rgb_led(LED2, RGB_YELLOW);
	set_rgb_led(LED4, RGB_YELLOW);
	set_rgb_led(LED6, RGB_YELLOW);
	set_rgb_led(LED8, RGB_YELLOW);

    chThdSleepSeconds(1);

    //IR
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    proximity_start();
    management_proximity_start();

	set_rgb_led(LED2, RGB_GREEN);
	set_rgb_led(LED4, RGB_GREEN);
	set_rgb_led(LED6, RGB_GREEN);
	set_rgb_led(LED8, RGB_GREEN);
	chThdSleepSeconds(1);

    //Transmission
//    management_transmissions_start();

    //DEV
    usb_start();
    serial_start();

    //Movement
    management_movement_start();

    /* Infinite loop. */
    while (1) {

    	chThdSleepMilliseconds(500);

    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
