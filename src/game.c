#include "stm32f0xx_hal.h"
#include "tft_display_driver.h"  // Include your specific TFT driver header

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define PIXEL_SIZE    10
#define STACK_START_X 100
#define STACK_START_Y (SCREEN_HEIGHT - PIXEL_SIZE)
#define STACK_MAX_HEIGHT (SCREEN_HEIGHT / PIXEL_SIZE)

volatile uint8_t game_running = 0;
volatile uint8_t button_pressed = 0;

typedef struct {
    int16_t x;
    int16_t y;
    int8_t direction;         
} Pixel;

Pixel moving_pixel;
int16_t stack_top_y = STACK_START_Y;
int16_t stack_x = STACK_START_X;

void SystemClock_Config(void);
void Error_Handler(void);
void EXTI0_1_IRQHandler(void);
void init_game(void);
void update_game(void);
void draw_pixel(Pixel *pixel, uint16_t color);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    
    // Initialize peripherals (buttons, TFT, etc.)
    tft_init();  // Initialize TFT display
    init_game();

    while (1) {
        if (game_running) {
            update_game();
        }
    }
}

void init_game(void) {
    game_running = 0;
    moving_pixel.x = STACK_START_X;
    moving_pixel.y = stack_top_y;
    moving_pixel.direction = 1;
    
    // Clear the display
    tft_fill_screen(BLACK);
    
    // Draw the initial pixel
    draw_pixel(&moving_pixel, WHITE);
}

void update_game(void) {
    if (button_pressed) {
        button_pressed = 0;

        // Check if the stack is aligned
        if (moving_pixel.x != stack_x) {
            // Game over
            tft_fill_screen(RED);  // Display game over indication
            HAL_Delay(2000);       // Delay for game over display
            init_game();           // Restart game
            return;
        }

        // Move the stack up
        stack_top_y -= PIXEL_SIZE;
        if (stack_top_y < 0) {
            stack_top_y = 0;
            game_running = 0;  // Game won condition (if needed)
        }
        stack_x = moving_pixel.x; // Update the stack position
        moving_pixel.y = stack_top_y;
    }

    // Move the pixel
    tft_fill_rect(moving_pixel.x, moving_pixel.y, PIXEL_SIZE, PIXEL_SIZE, BLACK);  // Erase old pixel
    moving_pixel.x += moving_pixel.direction * PIXEL_SIZE / 2;  // Adjust speed as needed

    // Boundary check
    if (moving_pixel.x <= 0 || moving_pixel.x >= (SCREEN_WIDTH - PIXEL_SIZE)) {
        moving_pixel.direction *= -1;  // Reverse direction
    }

    // Draw new pixel
    draw_pixel(&moving_pixel, WHITE);
}

void draw_pixel(Pixel *pixel, uint16_t color) {
    tft_fill_rect(pixel->x, pixel->y, PIXEL_SIZE, PIXEL_SIZE, color);
}

// Interrupt handler for button press (start/stop)
void EXTI0_1_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {  // Check which button was pressed
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
        if (!game_running) {
            game_running = 1;  // Start the game
        } else {
            button_pressed = 1;  // Stop the pixel stack
        }
    }
}