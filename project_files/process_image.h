#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define NOT_FOUND				1000

uint16_t get_peak_position(void);
void process_image_start(void);
void start_image_processing(void);
void stop_image_processing(void);
uint16_t get_line_position(void);

#endif /* PROCESS_IMAGE_H */
