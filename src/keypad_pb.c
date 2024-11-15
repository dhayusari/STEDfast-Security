#include "stm32f0xx.h"
#include <stdint.h>

void enable_keypad_ports();
<<<<<<< HEAD:src/keypad.c
void push_queue(int n);
char pop_queue();
void update_history(int c, int rows);
void drive_column(int c);
int read_rows();
char get_key_event(void);
char get_keypress();
void show_keys(void);
=======
void keypad();
void enterPassword();
void EXTI4_15_IRQHandler();
>>>>>>> 21a7f9802ccc9e8a219d55349305f131a6ce3310:src/keypad_pb.c

void enable_keypad_ports() {
    // Only enable port C for the keypad
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~0xffff;
    GPIOC->MODER |= 0x55 << (4*2);
    GPIOC->OTYPER &= ~0xff;
    GPIOC->OTYPER |= 0xf0;
    GPIOC->PUPDR &= ~0xff;
    GPIOC->PUPDR |= 0x55;
}
// 16 history bytes.  Each byte represents the last 8 samples of a button.
uint8_t hist[16];
char queue[2];  // A two-entry queue of button press/release events.
int qin;        // Which queue entry is next for input
int qout;       // Which queue entry is next for output

const char keymap[] = "DCBA#9630852*741";

void push_queue(int n) {
    queue[qin] = n;
    qin ^= 1;
}

<<<<<<< HEAD:src/keypad.c
char pop_queue() {
    char tmp = queue[qout];
    queue[qout] = 0;
    qout ^= 1;
    return tmp;
}

void update_history(int c, int rows)
{
    // We used to make students do this in assembly language.
    for(int i = 0; i < 4; i++) {
        hist[4*c+i] = (hist[4*c+i]<<1) + ((rows>>i)&1);
        if (hist[4*c+i] == 0x01)
            push_queue(0x80 | keymap[4*c+i]);
        if (hist[4*c+i] == 0xfe)
            push_queue(keymap[4*c+i]);
    }
}

void drive_column(int c)
{
    GPIOC->BSRR = 0xf00000 | ~(1 << (c + 4));
}

int read_rows()
{
    return (~GPIOC->IDR) & 0xf;
}

char get_key_event(void) {
    for(;;) {
        asm volatile ("wfi");   // wait for an interrupt
        if (queue[qout] != 0)
            break;
    }
    return pop_queue();
}

char get_keypress() {
    char event;
    for(;;) {
        // Wait for every button event...
        event = get_key_event();
        // ...but ignore if it's a release.
        if (event & 0x80)
            break;
    }
    return event & 0x7f;
}

void show_keys(void)
{
    char buf[] = "        ";
    for(;;) {
        char event = get_key_event();
        memmove(buf, &buf[1], 7);
        buf[7] = event;
        print(buf);
    }
}
=======
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
>>>>>>> 21a7f9802ccc9e8a219d55349305f131a6ce3310:src/keypad_pb.c
