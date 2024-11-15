#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();
void enable_sensor_ports();
void enable_sensor();
void disable_sensor();
void read_motion();
void update_hist_sensor();
void init_tim6();
void TIM6_DAC_IRQHandler();

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

void update_hist_sensor(){
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

void TIM6_DAC_IRQHandler() {
    TIM6 ->  SR &= ~TIM_SR_UIF;
    update_hist_sensor();
    read_motion();
}

void init_tim6(void) {
    RCC-> APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM6 -> PSC = 4800 - 1;
    TIM6-> ARR = 10 - 1;
    TIM6 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM6_DAC_IRQn;
    TIM6 -> CR1 |= TIM_CR1_CEN;
}