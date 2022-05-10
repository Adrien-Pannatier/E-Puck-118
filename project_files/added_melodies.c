/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Authors: Axel Praplan, Adrien Pannatier
 *
 *  Functions and defines to store and play custom melodies and alarms, using the play_melody.h library
 *
 *  adapted from the library code
 */

#include "added_melodies.h"

//conditional variable
static MUTEX_DECL(play_add_alarm_lock);
static CONDVAR_DECL(play_add_alarm_condvar);

//reference
static thread_reference_t play_add_alarm_ref = NULL;

//variable to stop the playing if necessary
static bool addplay = true;

/****************************NOTES LIBRARY*************************************/

//ROCKY melody
static const uint16_t rocky_melody[] = {
	NOTE_C4,

	NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4,
	NOTE_C4, NOTE_C4, NOTE_E4, NOTE_C4,
	NOTE_C4, NOTE_C4,NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_G4, NOTE_E4,
	NOTE_E4, NOTE_E4,

	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, 0, NOTE_D4,

	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, NOTE_D4, NOTE_D4, NOTE_E4,
	NOTE_G4,

	//REFRAIN
	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4,


	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
	NOTE_C4, NOTE_D4, NOTE_E4,

	NOTE_A3, NOTE_C4, NOTE_C4, NOTE_B3,
	NOTE_B3, NOTE_A3, NOTE_B3, NOTE_B3,
	NOTE_F4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_G4,

	//REFRAIN
	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A4, NOTE_A4, NOTE_B4,

	NOTE_E4,


	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
	NOTE_C4, NOTE_D4, NOTE_E4,

	NOTE_E4, NOTE_C4, NOTE_C4, NOTE_B4,
	NOTE_B4, NOTE_A4, NOTE_A4, NOTE_A4,
	NOTE_G4, NOTE_F4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A4, NOTE_G4, NOTE_A4, NOTE_C4,

	NOTE_A4,0
};

//GHOST_BUSTERS song
static const uint16_t ghost_buster_song[] = {
	NOTE_D4, NOTE_D4, NOTE_FS4, NOTE_D4, NOTE_E4,
	NOTE_C4, 0, 0, NOTE_D4, NOTE_D4, NOTE_D4,
	NOTE_D4, NOTE_C4, NOTE_D4, 0
};

//PINPON alarm
static const uint16_t pinpon_alarm[] = {
	NOTE_A4, NOTE_B4,NOTE_A4, NOTE_B4,
	NOTE_A4, NOTE_B4,
};

//SIREN_I alarm
static const uint16_t siren_I_alarm[] = {
	A_0, A_1, A_2, A_3, A_4, A_5, A_6, A_7,
	A_8, A_9, A_10, A_11, A_12, A_13, A_14,
	A_15, A_16, A_17, A_18, A_19, A_20, A_21,
	A_22, A_23,

	A_23, A_22, A_21, A_20, A_19, A_18, A_17,
	A_16, A_15, A_14, A_13, A_12, A_11, A_10,
	A_9, A_8, A_7, A_6, A_5, A_4, A_3, A_2, A_1,
	A_0,0
};

static const uint16_t baseball_alarm[] = {
	NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4,
	NOTE_E4, NOTE_G4, 0
};

/****************************TEMPO LIBRARY*************************************/

//ROCKY tempo
static const float rocky_tempo[] = {
	8,

	8, 16, 16, 8,
	16, 16, 8, 16,
	16, 8, 8,

	8, 16, 16, 8,
	16, 16, 8, 16,
	16, 4,

	8, 16, 16, 8,
	16, 16, 16, 16,
	8, 8, 8,

	8, 16, 16, 16,
	8, 16, 4, 16,
	16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1,

	8, 16, 16, 16/3,
	16, 16, 16/7,

	8, 16, 16, 8,
	16, 8, 16, 4,
	8,

	4, 4, 8, 8,
	8, 8,

	16, 8, 16, 8,
	16, 8, 16, 8,
	16, 16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1.3, 16, 16/3,

	1,

	8, 16, 16, 16/3,
	16, 16, 16/7,

	8, 16, 16, 8,
	16, 16, 16, 16,
	4, 8,

	4, 8, 8, 8,
	8, 8, 8,

	8, 8, 8, 8,
	4, 16, 16/3,

	4, 16, 16/3, 4,

	4/5,1

};

//GHOST_BUSTERS tempo
static const float ghost_buster_tempo[] = {
	16, 16, 8, 8, 8,
	4, 16, 16, 16, 16, 16,
	16, 8, 4, 4
};

static const float pinpon_tempo[] = {
	1,1,1,
	1,1,1
};

static const float siren_I_tempo[] = {
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2,
	2, 2,

	2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2,
	2,2
};

static const float baseball_tempo[] = {
	6, 6, 6, 8/3,
	8, 1, 1
};

/****************************MELODY LIBRARY*************************************/

static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),
  },
  //GHOST BUSTER
  {
    .notes = ghost_buster_song,
    .tempo = ghost_buster_tempo,
    .length = sizeof(ghost_buster_song)/sizeof(uint16_t),
  },
};

static const melody_t added_alarms[NB_ADDED_ALARMS] = {
  //PINPON
  {
    .notes = pinpon_alarm,
    .tempo = pinpon_tempo,
    .length = sizeof(pinpon_alarm)/sizeof(uint16_t),
  },
  //SIREN_INTERNATIONAL
  {
	.notes = siren_I_alarm,
	.tempo = siren_I_tempo,
	.length = sizeof(siren_I_alarm)/sizeof(uint16_t),
  },
  //BASEBALL
  {
	.notes = baseball_alarm,
	.tempo = baseball_tempo,
	.length = sizeof(baseball_alarm)/sizeof(uint16_t),
  }
};

static THD_WORKING_AREA(waPlayAddAlarmThd, 128);
static THD_FUNCTION(PlayAddAlarmThd, arg) {

  chRegSetThreadName("PlayAddAlarm Thd");

	(void)arg;

	static melody_t* alarm = NULL;

	while(1){
		//this thread is waiting until it receives a message
		chSysLock();
		alarm = (melody_t*) chThdSuspendS(&play_add_alarm_ref);
		chSysUnlock();

		for (int thisNote = 0; thisNote < alarm->length; thisNote++) {

      if(!addplay){
        break;
      }

			// to calculate the note duration, take one second
			// divided by the note type.
			//e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
			uint16_t noteDuration = (uint16_t)(1000 / alarm->tempo[thisNote]);

			playNote(alarm->notes[thisNote], noteDuration);

			// to distinguish the notes, set a minimum time between them.
			// the note's duration + 30% seems to work well:
//			uint16_t pauseBetweenNotes = (uint16_t)(noteDuration * 1.30);
//			chThdSleepMilliseconds(pauseBetweenNotes);

		}
    addplay = false;
    //signals to the threads waiting that the melody has finished
    chCondBroadcast(&play_add_alarm_condvar);
	}
}

/****************************PUBLIC FUNCTIONS*************************************/

void playAddedMelody(added_song_selection_t choice, play_melody_option_t option){
	playMelody(EXTERNAL_SONG, option, &added_melodies[choice]);
}

void playAddedAlarmStart(void){

	//create the thread
	chThdCreateStatic(waPlayAddAlarmThd, sizeof(waPlayAddAlarmThd), NORMALPRIO, PlayAddAlarmThd, NULL);
}

void playAddedAlarm(added_alarm_selection_t choice, play_melody_option_t option){

  melody_t* alarm = NULL;

  alarm = &added_alarms[choice];

  //SIMPLE_PLAY case
  if(option == ML_SIMPLE_PLAY){
    //if the reference is NULL, then the thread is already running
    //when the reference becomes not NULL, it means the thread is waiting
    if(play_add_alarm_ref != NULL){
      addplay = true;
      //tell the thread to play the alarm given
      chThdResume(&play_add_alarm_ref, (msg_t) alarm);
    }
  }//FORCE_CHANGE or WAIT_AND_CHANGE cases
  else{
    if(option == ML_FORCE_CHANGE){
      stopCurrentAlarm();
    }
    waitAlarmHasFinished();
    addplay = true;
    //tell the thread to play the alarm given
    chThdResume(&play_add_alarm_ref, (msg_t) alarm);
  }
}

void stopCurrentAlarm(void){
    addplay = false;
}

void waitAlarmHasFinished(void) {
  //if a melody is playing
  if(play_add_alarm_ref == NULL){
    //waits until the current melody is finished
    chMtxLock(&play_add_alarm_lock);
    chCondWait(&play_add_alarm_condvar);
    chMtxUnlock(&play_add_alarm_lock);
  }
}

/**************************END PUBLIC FUNCTIONS***********************************/
