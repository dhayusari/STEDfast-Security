#include "stm32f0xx_hal.h"

void init_spi1_slow(void) {
    // Enable clock for GPIOB and SPI1
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    
    // Configure GPIOB pins for SPI1
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure PB3 (SCK), PB4 (MISO), PB5 (MOSI)
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Configure SPI1
    SPI_HandleTypeDef hspi1 = {0};
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; // Maximum divisor (slow speed)
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        // Initialization error
        Error_Handler();
    }

    // Set FIFO reception threshold
    __HAL_SPI_ENABLE(&hspi1);
}

void enable_sdcard(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET); // Set PB2 low to enable SD card
}

void disable_sdcard(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET); // Set PB2 high to disable SD card
}

void init_sdcard_io(void) {
    // Initialize SPI1 in slow mode
    init_spi1_slow();

    // Configure PB2 as output for SD card enable/disable
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Disable SD card initially
    disable_sdcard();
}

void sdcard_io_high_speed(void) {
    // Disable SPI1
    SPI1->CR1 &= ~SPI_CR1_SPE;

    // Set SPI1 baud rate to 12 MHz (adjust as needed)
    SPI1->CR1 &= ~SPI_CR1_BR; // Clear baud rate bits
    SPI1->CR1 |= SPI_BAUDRATEPRESCALER_8; // Assuming 12 MHz SPI clock

    // Re-enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;
}

void init_lcd_spi(void) {
    // Configure PB8, PB11, PB14 as GPIO outputs
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_11 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Initialize SPI1 in slow mode
    init_spi1_slow();

    // Switch to high speed for LCD
    sdcard_io_high_speed();
}