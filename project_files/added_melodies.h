/*
 * added_melodies.h
 *
 *  Created on: 8 avr. 2022
 *      Author: Adrien
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

//available new alarms
typedef enum{
	//internal alarms available
	PINPON = 0,
	NB_ADDED_ALARMS,	//tell the number of internal alarms
}added_alarm_selection_t;

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

/**
 * @brief Starts the play_added_alarm module
 *
 */
void playAddedAlarmStart(void);

/**
 * @brief Plays the selected alarm. Does nothing if the module has not been started with playAddedAlarmStart()
 * 							This function doesn't block the current thread. It uses it's self thread
 *
 * @param choice 			Alarm selected (see added_alarm_selection_t)
 * @param option			Behavior to change the alarm playing (see play_melody_option_t)
 *
 */
void playAddedAlarm(added_alarm_selection_t choice, play_melody_option_t option);

/**
 * @brief 	Stops the Alarm beeing played. Even if this function returns immediatly,
 * 			the stopping of the playback is not immediate. It can take a very little time.
 * 			Use waitAlarmHasFinished() to be sure the sound has stopped.
 */
void stopCurrentAlarm(void);

/**
 * @brief Waits until the alarm playing has finished (put the invocking thread in sleep)
 * 		  Immediatly returns if no alarm is playing
 */
void waitAlarmHasFinished(void);

#endif /* ADDED_MELODIES_H_ */
