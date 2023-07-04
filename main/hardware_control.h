#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#define COMMAND_UP 1
#define COMMAND_DOWN 0
#define COMMAND_UNDEFINED -1
#include <stdint.h>

/*
gpios defines
*/
#define RELAY_UP_GPIO 19
#define RELAY_DOWN_GPIO 18
#define SW_UP_GPIO 27
#define SW_DOWN_GPIO 26


/*
function prototypes
*/
void init_blocator_gpios();
uint8_t execute_command_up();
uint8_t execute_command_down();
void execute_command_undef();
int get_motor_milliamps();
_Bool is_motor_blocked();
void on_movement_common();



//task function to be continously in hardware control
void hardware_control_fun(void *parameters);







#endif