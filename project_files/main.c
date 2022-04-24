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

#include <process_image.h>
#include <camera/po8030.h>

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

void SendUint8ToComputer(uint8_t* data, uint16_t size)
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

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
    halInit();
    chSysInit();
    mpu_init();
   // LED_start();
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    proximity_start();
    spi_comm_start();
    dac_start();
    playMelodyStart();

    //starts the camera
    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();
    dcmi_start();
    po8030_start();
    process_image_start();

    //start movement related thread
    management_transmissions_start();
    management_proximity_start();
    management_movement_start();

    messagebus_init(&bus, &bus_lock, &bus_condvar);

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
