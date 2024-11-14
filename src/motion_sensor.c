#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();
void enable_ports();
void enable_sensor();
void disable_sensor();
void read_motion();
void update_history();
void init_tim7();
void TIM7_IRQHandler();

//global variables
uint8_t hist; // 8 sample bits of input

void enable_sensor_ports(){
    // rcc for GPIOA
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    //PA5 for output & PA0 for input; PA3 is output
    GPIOA -> MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER0 | GPIO_MODER_MODER3);
    GPIOA -> MODER |= GPIO_MODER_MODER5_0 | GPIO_MODER_MODER3_0;
    GPIOA -> PUPDR |= GPIO_PUPDR_PUPDR0_1; // pull down resistor added
}
void enable_sensor(){
    //turning on pin
    GPIOA -> BSRR |= GPIO_BSRR_BS_5;
}

void disable_sensor(){
    //turning of PA5
    GPIOA -> BRR |= GPIO_BRR_BR_5;
}

void read_motion() {

    if (hist == 0x01){
        GPIOA -> BSRR |= GPIO_BSRR_BS_3;
    }
    if (hist == 0xfe)
         GPIOA -> BSRR |= GPIO_BSRR_BR_3;
    // else {
    //     GPIOA -> BSRR |= GPIO_BSRR_BR_3;
    // }
}

void update_history(){
    int temp = GPIOA -> IDR & 1;
    hist = (hist << 1) | temp;
    // if (temp) {
    //     hist = 255;
    // }
    // else if (hist == 0) {
    //     hist = 0;
    // }
    // else {
    //     hist--;
    // }
}

void TIM7_IRQHandler() {
    TIM7 ->  SR &= ~TIM_SR_UIF;
    update_history();
    read_motion();
}

void init_tim7(void) {
    RCC-> APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 -> PSC = 4800 - 1;
    TIM7-> ARR = 10 - 1;
    TIM7 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM7_IRQn;
    TIM7 -> CR1 |= TIM_CR1_CEN;
}