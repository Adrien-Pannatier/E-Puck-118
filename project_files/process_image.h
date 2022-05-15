/*
 * process_image.h
 *
 *  Created on: 18 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Thread and function to read images and detect high intensity peak
 *
 *  Inspired by the TP file
 */

#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define NOT_FOUND				1000

/**
 * @brief Activates the image thread
 *
 */
void process_image_start(void);

/**
 * @brief Enables image processing
 *
 */
void start_image_processing(void);

/**
 * @brief Stops image processing
 *
 */
void stop_image_processing(void);

/**
 * @brief return the position of the peak
 * 		  on the pixel line
 *
 * @return	int value of the position on the line
 *
 */
uint16_t get_fire_position(void);

#endif /* PROCESS_IMAGE_H */
