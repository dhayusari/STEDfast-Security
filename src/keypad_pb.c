#include "stm32f0xx.h"
#include <stdint.h>

void enable_keypad_ports();
void keypad();
void enterPassword();
void EXTI4_15_IRQHandler();

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

volatile uint32_t button_pressed = 0; // Variable to store the value when a button is pressed
  
void enterPassword() {
  //Using PC9-10 as inputs for push buttons
  RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
  RCC -> APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
  GPIOC -> MODER &= ~(GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
  GPIOC -> PUPDR |= (GPIO_PUPDR_PUPDR9_1 | GPIO_PUPDR_PUPDR10_1);

  SYSCFG -> EXTICR[2] |= 0x0020;
  SYSCFG -> EXTICR[2] |= 0x0200;

  EXTI -> RTSR |= (EXTI_RTSR_RT9 | EXTI_RTSR_RT10);
  EXTI -> IMR |= (EXTI_IMR_IM9 | EXTI_IMR_IM10);
  NVIC -> ISER[0] |= 1<<EXTI4_15_IRQn;
}

void EXTI4_15_IRQHandler(void) {
  if(EXTI -> PR & EXTI_PR_PR9) {
    EXTI -> PR |= EXTI_PR_PR9;
    button_pressed = 1;
  }

  if(EXTI -> PR & EXTI_PR_PR10) {
    EXTI -> PR |= EXTI_PR_PR10;
    button_pressed = 1;
  }
}
