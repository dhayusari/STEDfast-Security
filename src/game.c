#include "stm32f0xx.h"
#include <string.h> // memmove
#include <stdlib.h> // for srandom() and random()
#include <stdio.h>

void init_spi2(void);
void spi2_setup_dma(void);
void spi2_enable_dma(void);
void init_spi1(void);
void spi1_init_oled(void);
void spi1_setup_dma(void);
void spi1_enable_dma(void);
char get_keypress(); // Wait for key to start
void print(const char str[]);

extern uint16_t display[34];

void spi1_dma_display1(const char *str)
{
    for(int i=0; i<16; i++) {
        if (str[i])
            display[i+1] = 0x200 + str[i];
        else {
            // End of string.  Pad with spaces.
            for(int j=i; j<16; j++)
                display[j+1] = 0x200 + ' ';
            break;
        }
    }
}

void spi1_dma_display2(const char *str)
{
    for(int i=0; i<16; i++) {
        if (str[i])
            display[i+18] = 0x200 + str[i];
        else {
            // End of string.  Pad with spaces.
            for(int j=i; j<16; j++)
                display[j+18] = 0x200 + ' ';
            break;
        }
    }
}

int score = 0;
char disp1[17] = "                ";
char disp2[17] = "                ";
volatile int pos = 0;
void TIM17_IRQHandler(void)
{
    TIM17->SR &= ~TIM_SR_UIF;
    memmove(disp1, &disp1[1], 16);
    memmove(disp2, &disp2[1], 16);
    if (pos == 0) {
        if (disp1[0] != ' ')
            score -= 1;
        if (disp2[0] != ' ')
            score += 1;
        disp1[0] = '>';
    } else {
        if (disp2[0] != ' ')
            score -= 1;
        if (disp1[0] != ' ')
            score += 1;
        disp2[0] = '>';
    }
    int create = random() & 3;
    if (create == 0) { // one in four chance
        int line = random() & 1;
        if (line == 0) { // pick a line
            disp1[15] = 'x';
            disp2[15] = ' ';
        } else {
            disp1[15] = ' ';
            disp2[15] = 'x';
        }
    } else {
        disp1[15] = ' ';
        disp2[15] = ' ';
    }
    if (pos == 0)
        disp1[0] = '>';
    else
        disp2[0] = '>';
    if (score >= 100) {
        print("Score100");
        spi1_dma_display1("Game over");
        spi1_dma_display2("You win");
        NVIC->ICER[0] = 1<<TIM17_IRQn;
        return;
    }
    char buf[9];
    snprintf(buf, 9, "Score% 3d", score);
    print(buf);
    spi1_dma_display1(disp1);
    spi1_dma_display2(disp2);
    TIM17->ARR = 250 - 1 - 2*score;
}

void init_tim17(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
    TIM17->PSC = 48000 - 1;
    TIM17->ARR = 250 - 1;
    TIM17->CR1 |= TIM_CR1_ARPE;
    TIM17->DIER |= TIM_DIER_UIE;
    TIM17->CR1 |= TIM_CR1_CEN;
}


void game(void)
{
    print("Score  0");
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    spi1_dma_display1("Hit key to play");
    spi1_dma_display2("Hit A/B to move");
    init_spi1();
    spi1_init_oled();
    spi1_setup_dma();
    spi1_enable_dma();
    init_tim17(); // start timer
    get_keypress(); // Wait for key to start
    spi1_dma_display1(">               ");
    spi1_dma_display2("                ");
    // Use the timer counter as random seed...
    srandom(TIM17->CNT);
    // Then enable interrupt...
    NVIC->ISER[0] = 1<<TIM17_IRQn;
    for(;;) {
        char key = get_keypress();
        if (key == 'A' || key == 'B') {
            // If the A or B key is pressed, disable interrupts while
            // we update the display.
            asm("cpsid i");
            if (key == 'A') {
                pos = 0;
                disp1[0] = '>';
                disp2[0] = ' ';
            } else {
                pos = 1;
                disp1[0] = ' ';
                disp2[0] = '>';
            }
            spi1_dma_display1(disp1);
            spi1_dma_display2(disp2);
            asm("cpsie i");
        }
    }
}
