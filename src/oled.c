#include "stm32f0xx.h"

void set_char_msg(int, char);
void nano_wait_oled(unsigned int);
void game(void);
void internal_clock();
void check_wiring();
void autotest();



uint8_t col; // the column being scanned

void drive_column(int);   // energize one of the column outputs
int  read_rows();         // read the four row inputs
void update_history_oled(int col, int rows); // record the buttons of the driven column
char get_key_event(void); // wait for a button event (press or release)
char get_keypress(void);  // wait for only a button press event.
float getfloat(void);     // read a floating-point number from keypad
void show_keys(void);     // demonstrate get_key_event()

//===========================================================================
// Configure GPIOC 
//===========================================================================
void enable_ports(void) {
    // Only enable port C for the keypad
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~0xffff;
    GPIOC->MODER |= 0x55 << (4*2);
    GPIOC->OTYPER &= ~0xff;
    GPIOC->OTYPER |= 0xf0;
    GPIOC->PUPDR &= ~0xff;
    GPIOC->PUPDR |= 0x55;
}

//===========================================================================
// Bit Bang SPI LED Array
//===========================================================================
int msg_index = 0;
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
extern const char font[];


void small_delay(void) {
    nano_wait_oled(50000);
}

//============================================================================
// Configure Timer 15 for an update rate of 1 kHz.
// Trigger the DMA channel on each update.
// Copy this from lab 4 or lab 5.
//============================================================================
void init_tim15(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15->PSC = 479;
    TIM15->ARR = 99;
    TIM15->DIER |= TIM_DIER_UDE;
    TIM15->CR1 |= TIM_CR1_CEN;
}


//===========================================================================
// Configure timer 7 to invoke the update interrupt at 1kHz
// Copy from lab 4 or 5.
//===========================================================================
void init_tim7(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 479;
    TIM7->ARR = 99;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] = 1 << TIM7_IRQn;
    TIM7->CR1 |= TIM_CR1_CEN; 

}


//===========================================================================
// Copy the Timer 7 ISR from lab 5
//===========================================================================
// TODO To be copied
void TIM7_IRQHandler(){
    TIM7->SR &= ~TIM_SR_UIF;

    int rows = read_rows();
    update_history_oled(col, rows);
    col = (col + 1) & 3;
    drive_column(col);
}


//===========================================================================
// 4.4 SPI OLED Display
//===========================================================================
void init_spi1() {
    //enable clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Configure PA9, PA10, and PA11 as Alternate Function for NSS, SCK, and MOSI, respectively
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2 | GPIO_AFRH_AFRH3);
    GPIOA->AFR[1] |= (5 << GPIO_AFRH_AFRH1_Pos) | (5 << GPIO_AFRH_AFRH2_Pos) | (5 << GPIO_AFRH_AFRH3_Pos);
    
    SPI1 -> CR1 &= ~(SPI_CR1_SPE);
    SPI1 -> CR1 |= SPI_CR1_BR;
    SPI1 -> CR2 = SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI1 -> CR1 |= SPI_CR1_MSTR;
    SPI1 -> CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI1 -> CR2 |= SPI_CR2_TXDMAEN;
    SPI1 -> CR1 |= SPI_CR1_SPE;
    

}
void spi_cmd(unsigned int data) {
    while((SPI1->SR & SPI_SR_TXE) == 0);
    SPI1->DR = data;
    

}
void spi_data(unsigned int data) {
    spi_cmd(data | 0x200);
    
}
void spi1_init_oled() {
    nano_wait_oled(1000000);
    spi_cmd(0x38);
    spi_cmd(0x08);
    spi_cmd(0x01);
    nano_wait_oled(2000000);
    spi_cmd(0x06);
    spi_cmd(0x02);
    spi_cmd(0x0c);
    
}
void spi1_display1(const char *string) {
  spi_cmd(0x02);
  while (*string != '\0') {
    spi_data(*string);
    string++;
  }
    
}
void spi1_display2(const char *string) {
    spi_cmd(0xc0);
    while (*string != '\0') {
        spi_data(*string);
        string++;
    }
}

//===========================================================================
// This is the 34-entry buffer to be copied into SPI1.
// Each element is a 16-bit value that is either character data or a command.
// Element 0 is the command to set the cursor to the first position of line 1.
// The next 16 elements are 16 characters.
// Element 17 is the command to set the cursor to the first position of line 2.
//===========================================================================
uint16_t display[34] = {
        0x002, // Command to set the cursor at the first position line 1
        0x200+'E', 0x200+'C', 0x200+'E', 0x200+'3', 0x200+'6', + 0x200+'2', 0x200+' ', 0x200+'i',
        0x200+'s', 0x200+' ', 0x200+'t', 0x200+'h', + 0x200+'e', 0x200+' ', 0x200+' ', 0x200+' ',
        0x0c0, // Command to set the cursor at the first position line 2
        0x200+'c', 0x200+'l', 0x200+'a', 0x200+'s', 0x200+'s', + 0x200+' ', 0x200+'f', 0x200+'o',
        0x200+'r', 0x200+' ', 0x200+'y', 0x200+'o', + 0x200+'u', 0x200+'!', 0x200+' ', 0x200+' ',
};



//===========================================================================
// Main function
//===========================================================================
# define MAX_DIGITS 8

char entered_digits[MAX_DIGITS + 1] = {0};
int digit_index = 0;

void append_digit(char digit) {
    if (digit_index < MAX_DIGITS) {
        entered_digits[digit_index++] = digit;
        entered_digits[digit_index] = '\0';
    } else {
        // Handle overflow if necessary
    }
}


int main(void) {
    internal_clock();

    msg[0] |= font['E'];
    msg[1] |= font['C'];
    msg[2] |= font['E'];
    msg[3] |= font[' '];
    msg[4] |= font['3'];
    msg[5] |= font['6'];
    msg[6] |= font['2'];
    msg[7] |= font[' '];

    // GPIO enable
    enable_ports();
    // setup keyboard
    init_tim7();

    init_spi1();
    spi1_init_oled();

    while (1) {
        char key = get_keypress();
        if (key >= '0' && key <= '9') {
            append_digit(key);
            spi1_display1(entered_digits);
        }
    }

}