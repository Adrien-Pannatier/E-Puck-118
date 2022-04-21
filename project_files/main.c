#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "spi_comm.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <leds.h>
#include <msgbus/messagebus.h>
#include <audio/audio_thread.h>
#include <audio/play_melody.h>
#include <added_melodies.h>
#include "chmtx.h"
#include "light_gestion.h"
#include "audio/audio_thread.h"

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();
    messagebus_init(&bus, &bus_lock, &bus_condvar);
    proximity_start();
    spi_comm_start();
    dac_start();
    playMelodyStart();
    LED_start();

    /* Infinite loop. */
    while (1) {
    	chThdSleepMilliseconds(300);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
