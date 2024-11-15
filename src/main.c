#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();
void enable_sensor_ports();
void enable_keypad_ports();
void oled_main();


int main(void) {
    internal_clock();
    enable_sensor_ports();
<<<<<<< HEAD
    enable_keypad_ports();

    while(1) {
        oled_main();
    }
=======
    enable_sensor();
    init_tim7();
    //disable_sensor();
    // motion_sensor_dac();
    // init_tim6();
    oled_main();
>>>>>>> 5306035bb1e1ae611f1a40716f2bb5c410fbe8fb
}
