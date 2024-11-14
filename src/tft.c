#include "stm32f0xx.h"
#include <stdint.h>

void init_spi1_slow(){
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= ~(GPIO_MODER_MODER3 || GPIO_MODER_MODER4 || GPIO_MODER_MODER5);
    GPIOB -> MODER |= (GPIO_MODER_MODER3_0 || GPIO_MODER_MODER4_0 || GPIO_MODER_MODER5_0);
    GPIOB -> BSRR |= GPIO_BSRR_BR_3;
    
}
