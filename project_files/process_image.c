/*
 * process_image.c
 *
 *  Created on: 18 avr. 2022
 *     Authors: Axel Praplan, Adrien Pannatier
 *
 *  Thread and function to read images and detect high intensity peak
 *
 *  Inspired by the TP file
 */

#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>

#include "leds.h"


/* Constants for camera*/
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_FIRE_WIDTH			40
#define IMAGE_HEIGHT			2
#define IMAGE_X0				0
#define IMAGE_VERTICAL_POS		225

#define MASK_BLUE_L				0b00011111
#define MASK_GREEN_L   			0b11100000
#define MASK_GREEN_H			0b00000111
#define MASK_RED_H				0b11111000

#define FIRE_THRESHOLD 			50				//Threshold for fire detection
#define CERTAINTY_COUNTER		6


static uint16_t fire_position = NOT_FOUND;
static bool image_processing = false;
static bool image_capture = false;

//Private functions

/**
 * @brief 	Perform an image analysis to find if there is a fire
 *
 * @return	int value of the position on the fire, or NOT_FOUND if no fire was found
 *
 */
uint16_t detection_fire(uint8_t *buffer);

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg)
{
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels X0 to IMAGE_BUFFER_SIZE of the line IMAGE_VERTICAL_POS et IMAGE_VERTICAL_POS + 1 (minimum 2 fires because reasons)
	po8030_advanced_config(FORMAT_RGB565, IMAGE_X0, IMAGE_VERTICAL_POS, IMAGE_BUFFER_SIZE, IMAGE_HEIGHT, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1)
    {
    	if(image_capture == true)
    	{
			//starts a capture
			dcmi_capture_start();
			//waits for the capture to be done
			wait_image_ready();
			//signals an image has been captured
			chBSemSignal(&image_ready_sem);
    	}

    	chThdSleepMilliseconds(20);
    }
}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg)
{
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};
	uint8_t red_value;
	uint8_t green_value;
	uint8_t blue_value;
	bool one_on_two = false;

	uint16_t new_fire_position = NOT_FOUND;
	uint8_t counter = 0;

    while(1)
    {
    	if(image_processing == true)
    	{
			//waits until an image has been captured
			chBSemWait(&image_ready_sem);
			//gets the pointer to the array filled with the last image in RGB565
			img_buff_ptr = dcmi_get_last_image_ptr();

			//reading red pixel values
			one_on_two = !(one_on_two);

			if(one_on_two)
			{
				for(int i = 0; i < IMAGE_BUFFER_SIZE; i += 1)
				{
					//blue + green + red
					red_value = (img_buff_ptr[2*i] & MASK_RED_H)>>3;
					blue_value = img_buff_ptr[2*i+1] & MASK_BLUE_L;
					green_value  = (img_buff_ptr[2*i+1] & MASK_GREEN_L)>>5 | (img_buff_ptr[2*i] & MASK_GREEN_H)>>3;

					image[i] = red_value + green_value + blue_value;
				}
			}
			//Detection fire with certainty counter to assure no fire
			new_fire_position = detection_fire(image);

			if(new_fire_position == NOT_FOUND)
			{
				if(counter >= CERTAINTY_COUNTER)
				{
					fire_position = NOT_FOUND;
				}
				counter++;
			}
			else
			{
				fire_position = new_fire_position;
				counter = 0;
			}
    	}

    	chThdSleepMilliseconds(20);
    }
}

/***************************INTERNAL FUNCTIONS************************************/

uint16_t detection_fire(uint8_t *buffer)
{
	uint16_t i = 0, begin = 0, end = 0;
	bool stop = false, wrong_fire = false, fire_not_found = false;

	do
	{
		wrong_fire = false;
		//search for a begin
		while(stop == false && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			//the slope must at least be WIDTH_SLOPE wide and is compared
		    //to the mean of the image
		    if(buffer[i] < FIRE_THRESHOLD && buffer[i+WIDTH_SLOPE] > FIRE_THRESHOLD)
		    {
		        begin = i;
		        stop = 1;
		    }
		    i++;
		}
		//if a begin was found, search for an end
		if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
		{
		    stop = false;

		    while(stop == false && i < IMAGE_BUFFER_SIZE)
		    {
		        if(buffer[i] < FIRE_THRESHOLD && buffer[i-WIDTH_SLOPE] > FIRE_THRESHOLD)
		        {
		            end = i;
		            stop = 1;
		        }
		        i++;
		    }
		    //if an end was not found
		    if (i > IMAGE_BUFFER_SIZE || !end)
		    {
		        fire_not_found = 1;
		    }
		}
		else//if no begin was found
		{
		    fire_not_found = 1;
		}

		//if a fire too small has been detected, continues the search
		if(!fire_not_found && (end-begin) < MIN_FIRE_WIDTH)
		{
			i = end;
			begin = 0;
			end = 0;
			stop = false;
			wrong_fire = true;
		}
	}
	while(wrong_fire);

	if(fire_not_found)
	{
		begin = 0;
		end = 0;
		fire_position = NOT_FOUND;
	}
	else
	{
		fire_position = (begin + end)/2; //gives the fire position.
	}
	return fire_position;
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

void start_image_processing(void){
	image_processing = true;
	image_capture = true;
}

void stop_image_processing(void){
	image_processing = false;
	image_capture = false;
}

uint16_t get_fire_position(void){
	return fire_position;
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), LOWPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), LOWPRIO, CaptureImage, NULL);
}

/**************************END PUBLIC FUNCTIONS***********************************/
