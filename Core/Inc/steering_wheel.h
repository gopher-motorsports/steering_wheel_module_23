// GopherCAN_devboard_example.h
//  Header file for GopherCAN_devboard_example.c

#ifndef GOPHERCAN_DEVBOARD_EXAMPLE_H
#define GOPHERCAN_DEVBOARD_EXAMPLE_H

#define HEARTBEAT_MS_BETWEEN 500

#include "GopherCAN.h"

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
