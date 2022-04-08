/*
 * added_melodies.c
 *
 *  Created on: 8 avr. 2022
 *      Author: AdrienPannatier
 */

#include "added_melodies.h"

//ROCKY melody
static const uint16_t rocky_melody[] = {
	0, 0, 0, NOTE_C4,

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
	NOTE_A5, NOTE_A5, NOTE_B5,

	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A5, NOTE_A5, NOTE_B5,

	NOTE_E4,


	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
	NOTE_C4, NOTE_D4, NOTE_E4,

	NOTE_A4, NOTE_C4, NOTE_C4, NOTE_B4,
	NOTE_B4, NOTE_A4, NOTE_B4, NOTE_B4,
	NOTE_F4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_G4,

	//REFRAIN
	NOTE_A5, NOTE_A5, NOTE_B5,

	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A5, NOTE_A5, NOTE_B5,

	NOTE_E4,


	NOTE_C4, NOTE_D4, NOTE_C4, NOTE_D4,
	NOTE_C4, NOTE_D4, NOTE_E4,

	NOTE_E4, NOTE_C5, NOTE_C5, NOTE_B5,
	NOTE_B5, NOTE_A5, NOTE_A5, NOTE_A5,
	NOTE_G4, NOTE_F5,

	NOTE_E5, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_E4,

	NOTE_E4, NOTE_E4, NOTE_E4, NOTE_E4,
	NOTE_E4, NOTE_E4, NOTE_G4,

	NOTE_A5, NOTE_G4, NOTE_A5, NOTE_C5,

	NOTE_A5,
};

//ROCKY tempo
static const float rocky_tempo[] = {
	1/2, 1/4, 1/8, 1/8,

	1/8, 1/16, 1/16, 1/8,
	1/16, 1/16, 1/8, 1/16,
	1/16, 1/8, 1/8,

	1/8, 1/16, 1/16, 1/8,
	1/16, 1/16, 1/8, 1/16,
	1/16, 1/4,

	1/8, 1/16, 1/16, 1/8,
	1/16, 1/16, 1/16, 1/16,
	1/8, 1/8, 1/8,

	1/8, 1/16, 1/16, 1/16,
	1/8, 1/16, 1/4, 1/16,
	3/16,

	3/4, 1/16, 3/16,

	3/4, 1/16, 3/16,

	3/4, 1/16, 3/16,

	1,

	1/8, 1/16, 1/16, 3/16,
	1/16, 1/16, 7/16,

	1/8, 1/16, 1/16, 1/8,
	1/16, 2/16, 1/16, 1/4,
	1/8,

	1/4, 1/4, 1/8, 1/8,
	1/8, 1/8,

	1/16, 1/8, 1/16, 1/8,
	1/16, 2/16, 1/16, 1/8,
	1/16, 3/16,

	3/4, 1/16, 3/16,

	3/4, 1/16, 3/16,

	3/4, 1/16, 3/16,

	1,

	1/8, 1/16, 1/16, 3/16,
	1/16, 1/16, 7/16,

	1/8, 1/16, 1/16, 1/8,
	1/16, 1/16, 1/16, 1/16,
	1/4, 1/8,

	1/4, 1/8, 1/8, 1/8,
	1/8, 1/8, 1/8,

	1/8, 1/8, 1/8, 1/8,
	1/4, 1/16, 3/16,

	1/4, 1/16, 3/16, 1/4,

	5/4,

};

static const melody_t added_melodies[NB_ADDED_SONGS] = {
  //ROCKY
  {
    .notes = rocky_melody,
    .tempo = rocky_tempo,
    .length = sizeof(rocky_melody)/sizeof(uint16_t),
  },
};

void playAddedMelody(added_song_selection_t choice, play_melody_option_t option, float tempo_scale){
	choice.tempo = choice.tempo * tempo_scale;
	playMelody(EXTERNAL_SONG, option, choice);
}

