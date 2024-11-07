#include "stm32f0xx.h"
#include <stdint.h>

#define RATE 20000 // DAC trigger rate, can adjust as needed

void internal_clock();
void motion_sensor_adc(); // ADC input
void motion_sensor_dac(); // DAC output
void init_tim6(); // TIM6 for DAC trigger
void TIM6_DAC_IRQHandler(); 

void motion_sensor_adc() {
    // RCC for GPIOA
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    // PA0 for ADC_IN1
    GPIOA -> MODER |= 0xC;
    
    // Enable RCC for ADC
    RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN;
    
    // Enable 14 MHz HSI clock for ADC
    RCC -> CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY)); // Wait for HSI14 ready
    
    // Enable ADC
    ADC1 -> CR |= ADC_CR_ADEN;
    while (!(ADC1 -> ISR & ADC_ISR_ADRDY)); // Wait for ADC ready
    
    // Select channel 1 (ADC_IN1 at PA0)
    ADC1 -> CHSELR |= 1 << 1;
    while (!(ADC1 -> ISR & ADC_ISR_ADRDY)); // Wait for ADC ready again
}

void motion_sensor_dac() {
    // Enable GPIOA and set PA5 as analog for DAC output
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER |= 3 << (2 * 5); // PA5 to analog mode
    
    // Enable DAC clock
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;
    
    // Select TIM6 as trigger (TRGO)
    DAC->CR &= ~DAC_CR_TSEL1;
    
    // Enable DAC trigger and DAC channel 1
    DAC->CR |= DAC_CR_TEN1;
    DAC -> CR |= DAC_CR_EN1;
}

void init_tim6(void) {
    // Enable TIM6 clock
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    
    // Set the prescaler and ARR values to trigger DAC at RATE frequency
    TIM6 -> PSC = (48000000 / (RATE * 10)) - 1;
    TIM6 -> ARR = 10 - 1;

    // Configure TIM6 to generate update events as trigger for DAC
    TIM6 -> CR2 |= TIM_CR2_MMS_1; // Master Mode Select to Update
    
    // Enable update interrupt for TIM6
    TIM6 -> DIER |= TIM_DIER_UIE;

    // Enable TIM6 interrupt in NVIC
    NVIC -> ISER[0] = 1 << TIM6_DAC_IRQn;
    
    // Start TIM6
    TIM6 -> CR1 |= TIM_CR1_CEN;
}

void TIM6_DAC_IRQHandler() {
    // Clear the update interrupt flag
    if (TIM6 -> SR & TIM_SR_UIF) {
        TIM6 -> SR &= ~TIM_SR_UIF;
        
        // Set DAC to high output
        DAC -> DHR12R1 = 0xFFF; // Max value for 12-bit DAC (logic HIGH)
    }
}

int main(void) {
    internal_clock();
    motion_sensor_adc();
    motion_sensor_dac();
    init_tim6();

    // while (1) {
    //     // Check for motion detection from ADC
    //     if (/* Condition for motion detection, e.g., ADC1->DR > threshold */) {
    //         // Placeholder for command when motion is detected
    //     }
    // }
}
