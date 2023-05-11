// GopherCAN_devboard_example.c
//  This is a bare-bones module file that can be used in order to make a module main file

#include <steering_wheel.h>
#include "main.h"
#include "gopher_sense.h"
#include "GopherCAN.h"

// the HAL_CAN struct. This example only works for a single CAN bus
CAN_HandleTypeDef* example_hcan;


// Use this to define what module this board will be
#define THIS_MODULE_ID SWM_ID


// some global variables for examples
U8 last_button_state = 0;
U8 display_page = 1;


// the CAN callback function used in this example
static void change_led_state(U8 sender, void* UNUSED_LOCAL_PARAM, U8 remote_param, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3);
static void init_error(void);

BUTTON swUpshift = {
    .param = &swUpshift_state,
    .port = Up_Shift_In_GPIO_Port,
    .pin = Up_Shift_In_Pin
};

BUTTON swDownshift = {
    .param = &swDownshift_state,
    .port = Down_Shift_In_GPIO_Port,
    .pin = Down_Shift_In_Pin
};

BUTTON swFastClutch = {
    .param = &swFastClutch_state,
    .port = Fast_Clutch_In_GPIO_Port,
    .pin = Fast_Clutch_In_Pin
};

BUTTON swSlowClutch = {
    .param = &swSlowClutch_state,
    .port = Slow_Clutch_In_GPIO_Port,
    .pin = Slow_Clutch_In_Pin
};

BUTTON swButon0 = {
    .param = &swButon0_state,
    .port = Face_BTN0_In_GPIO_Port,
    .pin = Face_BTN0_In_Pin
};

BUTTON swButon1 = {
    .param = &swButon1_state,
    .port = Face_BTN1_In_GPIO_Port,
    .pin = Face_BTN1_In_Pin
};

BUTTON swButon2 = {
    .param = &swButon2_state,
    .port = Face_BTN2_In_GPIO_Port,
    .pin = Face_BTN2_In_Pin
};

BUTTON swButon3 = {
    .param = &swButon3_state,
    .port = Face_BTN3_In_GPIO_Port,
    .pin = Face_BTN3_In_Pin
};

BUTTON* buttons[NUM_OF_BUTTONS] = {
    &swUpshift,
    &swDownshift,
    &swFastClutch,
    &swSlowClutch,
	&swButon0,
    &swButon1,
    &swButon2,
    &swButon3,
};

// init
//  What needs to happen on startup in order to run GopherCAN
void init(CAN_HandleTypeDef* hcan_ptr)
{
	example_hcan = hcan_ptr;

	// initialize CAN
	// NOTE: CAN will also need to be added in CubeMX and code must be generated
	// Check the STM_CAN repo for the file "F0xx CAN Config Settings.pptx" for the correct settings
	if (init_can(GCAN0, example_hcan, THIS_MODULE_ID, BXTYPE_MASTER))
	{
		init_error();
	}

	// Set the function pointer of SET_LED_STATE. This means the function change_led_state()
	// will be run whenever this can command is sent to the module
	if (add_custom_can_func(SET_LED_STATE, &change_led_state, TRUE, NULL))
	{
		init_error();
	}

	// lock param sending for all of the buttons
	for (U8 i = 0; i < NUM_OF_BUTTONS; i++) {
	    lock_param_sending(&buttons[i]->param->info);
	}

	lock_param_sending(&swDial_ul.info);
}


// can_buffer_handling_loop
//  This loop will handle CAN RX software task and CAN TX hardware task. Should be
//  called every 1ms or as often as received messages should be handled
void can_buffer_handling_loop()
{
	// handle each RX message in the buffer
	if (service_can_rx_buffer())
	{
		// an error has occurred
	}

	// handle the transmission hardware for each CAN bus
	service_can_tx(example_hcan);
}

U8 button_state;
U8 last_displayPageButton_state;

U8 rot_sw0_in;
U8 rot_sw1_in;
U8 rot_sw2_in;
U8 rot_sw3_in;

//will store the bitwise version of all rotary inputs, Ex: rot0 = 1, rot1 = 1, rot2 = 1, and rot3 =1  --> rot_rawResult = 00001111
U8 rot_result;


// main_loop
//  another loop. This includes logic for sending a CAN command. Designed to be
//  called every 10ms
void main_loop()
{
	static U32 lastHeartbeat = 0;

	//reading in steering wheel buttons through GPIO pots
	for (U8 i = 0; i < NUM_OF_BUTTONS; i++) {
	    BUTTON* btn = buttons[i];
	    U8 new_state = !HAL_GPIO_ReadPin(btn->port, btn->pin);
	    if (new_state != btn->param->data) {
			// button state has changed, send message immediately
			send_parameter(&btn->param->info);
	    }
	    btn->param->data = new_state;
    }

	U8 new_displayPageButton_state = !HAL_GPIO_ReadPin(DISPLAY_PAGE_CHANGE_BUTTON.port, DISPLAY_PAGE_CHANGE_BUTTON.pin);
	if(new_displayPageButton_state > last_displayPageButton_state) {
		display_page = (display_page + 1) % (NUM_DISPLAY_PAGES);
	}
	last_displayPageButton_state = new_displayPageButton_state;

	update_and_queue_param_u8(&displayPage_state, display_page + 1);

	rot_sw0_in = HAL_GPIO_ReadPin(Rot_SW0_In_GPIO_Port, Rot_SW0_In_Pin);
	rot_sw1_in = HAL_GPIO_ReadPin(Rot_SW1_In_GPIO_Port, Rot_SW1_In_Pin);
	rot_sw2_in = HAL_GPIO_ReadPin(Rot_SW2_In_GPIO_Port, Rot_SW2_In_Pin);
	rot_sw3_in = HAL_GPIO_ReadPin(Rot_SW3_In_GPIO_Port, Rot_SW3_In_Pin);

	//performing bitwise operations to read in rotary positons
	//left to right is increasing, restarts to 0 at end of range
	rot_result = (rot_sw3_in << 2) | (rot_sw2_in << 3) | (rot_sw1_in << 1) | rot_sw0_in;
	rot_result = 15 - rot_result;

	update_and_queue_param_u8(&swDial_ul, rot_result);

	if (HAL_GetTick() - lastHeartbeat > HEARTBEAT_MS_BETWEEN)
	{
		lastHeartbeat = HAL_GetTick();
		HAL_GPIO_TogglePin(HBEAT_LED_GPIO_Port, HBEAT_LED_Pin);
	}
}


// can_callback_function example

// change_led_state
//  a custom function that will change the state of the LED specified
//  by parameter to remote_param. In this case parameter is a U16*, but
//  any data type can be pointed to, as long as it is configured and casted
//  correctly
static void change_led_state(U8 sender, void* parameter, U8 remote_param, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3)
{
	//HAL_GPIO_WritePin(GRN_LED_GPIO_Port, GRN_LED_Pin, !!remote_param);
	return;
}


// init_error
//  This function will stay in an infinite loop, blinking the LED in a 0.5sec period. Should only
//  be called from the init function before the RTOS starts
void init_error(void)
{
	while (1)
	{
		//HAL_GPIO_TogglePin(GRN_LED_GPIO_Port, GRN_LED_Pin);
		HAL_Delay(250);
	}
}

// end of GopherCAN_example.c
