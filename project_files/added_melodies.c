/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Author: AdrienPannatier
 */

#include "added_melodies.h"
#include "leds.h"

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

	3, 16, 16/3,

	3, 16, 16/3,

	3, 16, 16/3,

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

	3, 16, 16/3,

	3, 16, 16/3,

	3, 16, 16/3,

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




static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),
  },
};

void playAddedMelody(added_song_selection_t choice, play_melody_option_t option, uint16_t tempo_music){
	//if(sizeof(rocky_melody)/sizeof(uint16_t)==sizeof(rocky_tempo)/sizeof(float))set_led(LED1,1);
	playMelody(EXTERNAL_SONG, option, &added_melodies[choice]);
}

