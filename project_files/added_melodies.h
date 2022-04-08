/*
 * added_melodies.h
 *
 *  Created on: 8 avr. 2022
 *      Author: adrie
 */

#ifndef ADDED_MELODIES_H_
#define ADDED_MELODIES_H_

#include <audio/play_melody.h>

//available new songs
typedef enum{
	//internal songs available
	ROCKY = 0,
	NB_ADDED_SONGS,	//tell the number of internal songs
}added_song_selection_t;


/**
 * @brief Plays the selected melody from the external library. Does nothing if the module has not been
 * 							 started with playMelodyStart()	This function doesn't block the current thread.
 * 							 It uses it's self thread
 *
 * @param choice 			Song selected (see added_song_selection_t)
 * @param option			Behavior to change the melody playing (see play_melody_option_t)
 *
 */
void playAddedMelody(added_song_selection_t choice, play_melody_option_t option);

#endif /* ADDED_MELODIES_H_ */
