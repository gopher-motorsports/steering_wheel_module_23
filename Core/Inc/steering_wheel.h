// GopherCAN_devboard_example.h
//  Header file for GopherCAN_devboard_example.c

#ifndef GOPHERCAN_DEVBOARD_EXAMPLE_H
#define GOPHERCAN_DEVBOARD_EXAMPLE_H

#define HEARTBEAT_MS_BETWEEN 500
#define NUM_DISPLAY_PAGES 4

#include "GopherCAN.h"
#include "gopher_sense.h"

#define DISPLAY_PAGE_CHANGE_BUTTON (swButon0_state)

void init(CAN_HandleTypeDef* hcan_ptr);
void can_buffer_handling_loop();
void main_loop();

typedef struct {
    U8_CAN_STRUCT* param;
    GPIO_TypeDef* port;
    U16 pin;
} BUTTON;

#define NUM_OF_BUTTONS 8

#endif
