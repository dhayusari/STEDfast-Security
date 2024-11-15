#include "stm32f0xx.h"
#include <string.h> // memmove
#include <stdlib.h> // for srandom() and random()
#include <stdio.h>

<<<<<<< HEAD
//void set_char_msg(int, char);
void nano_wait(unsigned int n);
//void game(void);
=======
void set_char_msg(int, char);
void nano_wait_oled(unsigned int);
void game(void);
>>>>>>> 5306035bb1e1ae611f1a40716f2bb5c410fbe8fb
void internal_clock();
void enable_keypad_ports();
void enable_sensor();
void disable_sensor();
void init_tim6();
void oled_main(void);

uint8_t col; // the column being scanned

void drive_column(int);   // energize one of the column outputs
int  read_rows();         // read the four row inputs
void update_history_oled(int col, int rows); // record the buttons of the driven column
char get_key_event(void); // wait for a button event (press or release)
char get_keypress(void);  // wait for only a button press event.
void show_keys(void);     // demonstrate get_key_event()

// spi and dma
void init_spi2(void);
void spi2_setup_dma(void);
void spi2_enable_dma(void);
void init_spi1(void);
void spi1_init_oled(void);
void spi1_setup_dma(void);
void spi1_enable_dma(void);

const char font[] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, // 32: space
    0x86, // 33: exclamation
    0x22, // 34: double quote
    0x76, // 35: octothorpe
    0x00, // dollar
    0x00, // percent
    0x00, // ampersand
    0x20, // 39: single quote
    0x39, // 40: open paren
    0x0f, // 41: close paren
    0x49, // 42: asterisk
    0x00, // plus
    0x10, // 44: comma
    0x40, // 45: minus
    0x80, // 46: period
    0x00, // slash
    // digits
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67,
    // seven unknown
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    // Uppercase
    0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x6f, 0x76, 0x30, 0x1e, 0x00, 0x38, 0x00,
    0x37, 0x3f, 0x73, 0x7b, 0x31, 0x6d, 0x78, 0x3e, 0x00, 0x00, 0x00, 0x6e, 0x00,
    0x39, // 91: open square bracket
    0x00, // backslash
    0x0f, // 93: close square bracket
    0x00, // circumflex
    0x08, // 95: underscore
    0x20, // 96: backquote
    // Lowercase
    0x5f, 0x7c, 0x58, 0x5e, 0x79, 0x71, 0x6f, 0x74, 0x10, 0x0e, 0x00, 0x30, 0x00,
    0x54, 0x5c, 0x73, 0x7b, 0x50, 0x6d, 0x78, 0x1c, 0x00, 0x00, 0x00, 0x6e, 0x00
};

int msg_index = 0;
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
int sensor = 0;

// Turn on the dot of the rightmost display element.
// void dot()
// {
//     msg[7] |= 0x80;
// }
void small_delay(void) {
    nano_wait_oled(50000);
}

void init_tim7(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 479;
    TIM7->ARR = 99;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC->ISER[0] = 1 << TIM7_IRQn;
    TIM7->CR1 |= TIM_CR1_CEN; 

}

void TIM7_IRQHandler(){
    TIM7->SR &= ~TIM_SR_UIF;

    int rows = read_rows();
    update_history_oled(col, rows);
    col = (col + 1) & 3;
    drive_column(col);
}

void init_spi1() {
    //enable clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    
    //GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    //GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);
    //GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2 | GPIO_AFRH_AFRH3);
    //GPIOA->AFR[1] |= (5 << GPIO_AFRH_AFRH1_Pos) | (5 << GPIO_AFRH_AFRH2_Pos) | (5 << GPIO_AFRH_AFRH3_Pos);

    GPIOA -> MODER &= ~(0xC000CC00);
    GPIOA -> MODER |= 0x80008800;
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH7);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL5 | GPIO_AFRL_AFRL7);

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
    spi_cmd(data |0x200);
    
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

uint16_t display[34] = {
        0x002, // Command to set the cursor at the first position line 1
        0x200+'E', 0x200+'C', 0x200+'E', 0x200+'3', 0x200+'6', + 0x200+'2', 0x200+' ', 0x200+'i',
        0x200+'s', 0x200+' ', 0x200+'t', 0x200+'h', + 0x200+'e', 0x200+' ', 0x200+' ', 0x200+' ',
        0x0c0, // Command to set the cursor at the first position line 2
        0x200+'c', 0x200+'l', 0x200+'a', 0x200+'s', 0x200+'s', + 0x200+' ', 0x200+'f', 0x200+'o',
        0x200+'r', 0x200+' ', 0x200+'y', 0x200+'o', + 0x200+'u', 0x200+'!', 0x200+' ', 0x200+' ',
};

void print(const char str[])
{
    const char *p = str;
    for(int i=0; i<8; i++) {
        if (*p == '\0') {
            msg[i] = (i<<8);
        } else {
            msg[i] = (i<<8) | font[*p & 0x7f] | (*p & 0x80);
            p++;
        }
    }
}

// spi 2 dma
void spi2_enable_dma(void) {
    DMA1_Channel5 -> CCR |= DMA_CCR_EN; // enable
}

void spi1_setup_dma(void) {
    RCC ->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3 -> CCR &= ~DMA_CCR_EN; // turn off DMA
    DMA1_Channel3 -> CMAR = (uint32_t) display; // copy from address from CMAR
    DMA1_Channel3 -> CPAR = (uint32_t) &(SPI1 -> DR); // copy to address from CPAR
    DMA1_Channel3 -> CNDTR = 34; // amount of elements in display
    DMA1_Channel3 -> CCR |= DMA_CCR_DIR; // reading from memory
    DMA1_Channel3 -> CCR |= DMA_CCR_MINC; // incrementing CMAR
    DMA1_Channel3 -> CCR |= DMA_CCR_MSIZE_0; // 01: 16 bit
    DMA1_Channel3 -> CCR |= DMA_CCR_PSIZE_0; // 01: 16 bit
    DMA1_Channel3 -> CCR |= DMA_CCR_CIRC; // circular operation
    SPI1 -> CR2 |= SPI_CR2_TXDMAEN;
}

void spi1_enable_dma(void) {
     DMA1_Channel3 -> CCR |= DMA_CCR_EN; // enable
}

void init_spi2(void) {
    //enable clock for SPI and GPIOB
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    
    // alternate functions for GPIOB
    GPIOB -> MODER &= ~(0xCF000000);
    GPIOB -> MODER |= 0x8A000000;
    GPIOB -> AFR[1] &= ~(0xF0FF0000);

    // CR1 SPE bit is cleared
    SPI2 -> CR1 &= ~(SPI_CR1_SPE);
    // set baud rate as low as possible
    SPI2 -> CR1 |= SPI_CR1_BR;
    // configure for 16 bit word size
    SPI2 -> CR2 |= SPI_CR2_DS;
    // configure to be in master
    SPI2 -> CR1 |= SPI_CR1_MSTR;
    // set the SS ouput enable bit and enable nssp
    SPI2 -> CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP;
    // set TXDMAEN to enable dma transfers
    SPI2 -> CR2 |= SPI_CR2_TXDMAEN;
    // enable SPI channel
    SPI2 -> CR1 |= SPI_CR1_SPE;

}


void spi2_setup_dma(void) {
    RCC ->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5 -> CCR &= ~DMA_CCR_EN; // turn off DMA
    DMA1_Channel5 -> CMAR = (uint32_t) &msg; // copy from address from CMAR
    DMA1_Channel5 -> CPAR = (uint32_t) &(SPI2 -> DR); // copy to address from CPAR
    DMA1_Channel5 -> CNDTR = 8; // amount of LEDs
    DMA1_Channel5 -> CCR |= DMA_CCR_DIR; // reading from memory
    DMA1_Channel5 -> CCR |= DMA_CCR_MINC; // incrementing CMAR
    DMA1_Channel5 -> CCR |= DMA_CCR_MSIZE_0; // 01: 16 bit
    DMA1_Channel5 -> CCR |= DMA_CCR_PSIZE_0; // 01: 16 bit
    DMA1_Channel5 -> CCR |= DMA_CCR_CIRC; // circular operation
    SPI2 -> CR2 |= SPI_CR2_TXDMAEN;
}

# define MAX_DIGITS 4

//predefined passcode
const char predefined_passcode[] = "1234";


char entered_digits[MAX_DIGITS + 1] = {0};
int digit_index = 0;

void append_digit(char digit) {
    if (digit_index < MAX_DIGITS) {
        entered_digits[digit_index++] = digit;
        entered_digits[digit_index] = '\0';
    }
}

int check_passcode() {
    if (strcmp(entered_digits, predefined_passcode) == 0){
        spi1_display2("MATCHED!");
        return 1;
    }
    else{
        spi1_display2("INCORRECT");
        return 0;
    }
}

void reset_passcode_entry(void) {
    //clear entered_digits for new attempt
    digit_index = 0;
    entered_digits[0] = '\0';
}

<<<<<<< HEAD
void oled_main(void) {
 
=======
void clear_display(void) {
    spi_cmd(0x01); // Command to clear display
    nano_wait_oled(2000000); // Wait for the clear command to complete
}

void alarm(void){
    clear_display();
    spi1_display1("ALARMMM");
}

int oled_main(void) {
    internal_clock();

    enable_ports();
    init_tim7();
>>>>>>> 5306035bb1e1ae611f1a40716f2bb5c410fbe8fb
    init_spi1();
    spi1_init_oled();
    spi1_display1("Enter passcode:");

    int attempts = 0; //counter
    #define MAX_ATTEMPTS 3 //after max reached exit to alarm

    while(attempts < MAX_ATTEMPTS){
        char key = get_keypress();

        if (key >= '0' && key <= '9'){
            append_digit(key);
            spi1_display2(entered_digits);
        }
        else if (key == '#'){
            if (check_passcode()){
<<<<<<< HEAD
                spi1_display1("Matched");
                if (sensor == 0){
                    enable_sensor();
                    init_tim6();
                    sensor = 1;
                }
                else {
                    disable_sensor();
                    sensor = 0;
                }
                
=======
                break;
>>>>>>> 5306035bb1e1ae611f1a40716f2bb5c410fbe8fb
            }
            else{
                clear_display();
                spi1_display1("Re-Enter Code");
                attempts++;

            }
            digit_index = 0;
            entered_digits[0] = '\0';
            nano_wait_oled(3000000000);
        }
<<<<<<< HEAD
}
=======
    }
    if (attempts == MAX_ATTEMPTS){
        alarm();
    }

//implement 3 attempts
//need to change support.c files to avoid overlapping func names
//make it work on other board
}
>>>>>>> 5306035bb1e1ae611f1a40716f2bb5c410fbe8fb
