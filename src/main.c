#include "stm32f0xx.h"
#include <stdint.h>

void internal_clock();

void motion_sensor();

void motion_sensor(){
    // RCC for GPIOA
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    // PA0 for ADC_IN1
    GPIOA -> MODER |= 0xC;
    // enabling RCC for ADC
    RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN;
    // enabling 14 MHz 
    RCC -> CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY)); 
    //enabling ADC
    ADC1 -> CR |= ADC_CR_ADEN;
    while (!(ADC1 -> ISR & ADC_ISR_ADRDY)); // waiting for ADC to be ready
    ADC1 -> CHSELR |= 1 << 1; // channel 1
    while (!(ADC1 -> ISR & ADC_ISR_ADRDY)); // waiting for ADC to be ready
}