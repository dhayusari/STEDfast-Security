#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();
void enable_sensor_ports();
void enable_sensor();
void disable_sensor();
void update_history();
void read_motion();
void init_tim7();
void enable_keypad_ports();
void keypad();



int main(void) {
    internal_clock();
    enable_sensor_ports();
    enable_sensor();
    init_tim7();
    //disable_sensor();
    // motion_sensor_dac();
    // init_tim6();
}
