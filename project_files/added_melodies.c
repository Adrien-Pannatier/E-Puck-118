/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Author: AdrienPannatier
 *
 *  Functions and defines to store and play custom melodies and alarms, using the play_melody.h library
 *
 *  adapted from the library code
 */

#include "added_melodies.h"

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

	NOTE_A4,
};

//PINPON melody
static const uint16_t pinpon_melody[] = {
	NOTE_A5, NOTE_B5,NOTE_A5, NOTE_B5,
	NOTE_A5, NOTE_B5,NOTE_A5, NOTE_B5,
};

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

	4/5,

};

static const float pinpon_tempo[] = {
		8,8,8,8,
		8,8,8,8,


//	1,1,1,1,
//	1,1,1,1,
};

static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),
  },
  //PINPON
  {
    .notes = pinpon_melody,
    .tempo = pinpon_tempo,
    .length = sizeof(pinpon_melody)/sizeof(uint16_t),
  },
};

void playAddedMelody(added_song_selection_t choice, play_melody_option_t option){
	//if(sizeof(rocky_melody)/sizeof(uint16_t)==sizeof(rocky_tempo)/sizeof(float))set_led(LED1,1);
	playMelody(EXTERNAL_SONG, option, &added_melodies[choice]);
}

