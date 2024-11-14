#include "stm32f0xx.h"
#include <stdint.h>

void enable_keypad_ports();
void keypad();

void enable_keypad_ports() {
    //enabling RCC
    RCC ->AHBENR |= RCC_AHBENR_GPIOCEN;

    // clearing the bits
    GPIOC -> MODER &= ~(0x0000FFFF);

    // setting pin 4-7 as outputs;
    GPIOC -> MODER |= 0x00005500;

    //setting pin 0-3 as inputs with pull downs
    GPIOC -> PUPDR |= 0x000000AA;
}

void keypad(void){
  for (int i = 1; i<= 4; i++) {
    if (i == 1) {
      GPIOC -> ODR &= ~(0x1 << 7);
    }
    else {
      GPIOC -> ODR &= ~(0x1 << (i + 2));
    }

    GPIOC -> ODR |= ( 1 << (i + 3));
    // nano_wait(1000000);
    // int32_t rows = GPIOC -> IDR & 0xF;
    
    // if (rows == 0b0001) {
    //   setn(8, 1);
    // }
    // else if (rows == 0b0010) {
    //   setn(9, 1);
    // }
    // else if (rows == 0b0100) {
    //   setn(10, 1);
    // }
    // else if (rows == 0b1000) {
    //   setn(11, 1);
    // }
    // else {
    //   setn(8, 0);
    //   setn(9, 0);
    //   setn(10, 0);
    //   setn(11, 0);
    // }
    
  }
}

