/**
 * @file main.c
 * @brief Main source code for the Motor Control program.
 *
 * This file contains the main entry point and function definitions for the Motor Control program.
 *
 * SysTick is used to check if a collision has been detected and toggles the LEDs on the chassis board.
 *
 * It also uses Timer A0 to generate PWM signals which will be used to drive the DC motors.
 * Then, it uses edge-triggered interrupts from the bumper switches to detect a collision.
 * After a collision has been detected, the motors should stop from running.
 *
 * In addition, Timer A2 is used to generate PWM signals and drive two HS-485HB servos.
 *
 * @author
 */

#include <stdint.h>
#include "msp.h"
#include "inc/Clock.h"
#include "inc/CortexM.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/GPIO.h"
#include "inc/SysTick_Interrupt.h"
#include "inc/Timer_A2_PWM.h"
#include "inc/Bumper_Switches.h"
#include "inc/Motor.h"

// Initialize a global variable for SysTick to keep track of elapsed time in milliseconds
uint32_t SysTick_ms_elapsed = 0;

// Global flag that gets set in Bumper_Switches_Handler.
// This is used to detect if any collisions occurred when any one of the bumper switches are pressed.
volatile uint8_t collision_detected = 0;

static uint8_t Delay_Until_Collision(uint32_t duration_ms, uint8_t abort_on_collision)
{
    while (duration_ms > 0)
    {
        Clock_Delay1ms(1);
        if (abort_on_collision && (collision_detected != 0))
        {
            return 1;
        }
        duration_ms--;
    }

    return 0;
}

/**
 * @brief Interrupt service routine for the SysTick timer.
 *
 * The interrupt service routine for the SysTick timer increments the SysTick_ms_elapsed
 * global variable to keep track of the elapsed milliseconds. If collision_detected is 0, then
 * it checks if 500 milliseconds passed. It toggles the front yellow LEDs and turns off the back red LEDs
 * on the chassis board. Otherwise, if collision_detected is set, it turns off the front yellow LEDs
 * and turns on the back red LEDs on the chassis board.
 *
 * @param None
 *
 * @return None
 */
void SysTick_Handler(void)
{
    SysTick_ms_elapsed++;

    if (collision_detected == 0)
    {
        if (SysTick_ms_elapsed >= 500)
        {
            P8->OUT &= ~0xC0;
            P8->OUT ^= 0x21;
            SysTick_ms_elapsed = 0;
        }
    }

    else
    {
        P8->OUT |= 0xC0;
        P8->OUT &= ~0x21;
    }
}

/**
 * @brief Bumper switch interrupt handler function.
 *
 * This is the interrupt handler for the bumper switch interrupts. It is called when a falling edge event is detected on
 * any of the bumper switch pins. The function checks if a collision has already been detected; if not, it prints a collision
 * detection message along with the bumper switch state and sets the collision_detected flag to prevent further detections.
 *
 * @param bumper_switch_state An 8-bit unsigned integer representing the bumper switch states at the time of the interrupt.
 *
 * @return None
 */
void Bumper_Switches_Handler(uint8_t bumper_switch_state)
//set the P8.7 pin high when any of the bumper switches are pressed (indicating a collision)
{
    if (collision_detected == 0)
    {
        printf("Collision Detected! Bumper Switch State: 0x%02X\n", bumper_switch_state);
        P8->OUT |= 0x80; // Set P8.7 high to indicate collision
        collision_detected = 1; // Set the collision_detected flag
    }
}

/**
 * @brief Execute a predefined drive pattern using the DC motors.
 *
 * This function executes a predefined drive pattern using the DC motors. It involves a sequence of motor commands
 * to create specific movements. The sequence consists of:
 *
 * 1. Setting both motors to move forward with a 50% duty cycle for a duration of 2 seconds.
 * 2. Stopping the motors for 2 seconds.
 * 3. Setting both motors to move left with a 30% duty cycle for 2 seconds.
 * 4. Stopping the motors for 2 seconds.
 * 5. Setting both motors to move right with a 30% duty cycle for 2 seconds.
 * 6. Stopping the motors for 2 seconds.
 * 7. Setting both motors to move backward with a 30% duty cycle for 2 seconds.
 * 8. Stopping the motors for 2 seconds.
 *
 * @note The Clock_Delay1ms function is used to introduce delays between motor actions.
 *
 * @param None
 *
 * @return None
 */
void Drive_Pattern_1()
{
    // Set PWM to 50% Duty Cycle
    Motor_Forward(7500, 7500);
    if (Delay_Until_Collision(2000, 1)) { Motor_Stop(); return; }

    // Stop the motors
    Motor_Stop();
    if (Delay_Until_Collision(2000, 1)) { return; }

    // Set PWM to 30% Duty Cycle
    Motor_Left(4500, 4500);
    if (Delay_Until_Collision(2000, 1)) { Motor_Stop(); return; }

    // Stop the motors
    Motor_Stop();
    if (Delay_Until_Collision(2000, 1)) { return; }

    // Set PWM to 30% Duty Cycle
    Motor_Right(4500, 4500);
    if (Delay_Until_Collision(2000, 1)) { Motor_Stop(); return; }

    // Stop the motors
    Motor_Stop();
    if (Delay_Until_Collision(2000, 1)) { return; }

    // Set PWM to 30% Duty Cycle
    Motor_Backward(4500, 4500);
    if (Delay_Until_Collision(2000, 1)) { Motor_Stop(); return; }

    // Stop the motors
    Motor_Stop();
    Delay_Until_Collision(2000, 1);
}

/**
 * @brief
 *
 * @param None
 *
 * @return None
 */
void Handle_Collision()
{
    // Stop the motors
    Motor_Stop();


    // Make a function call to Clock_Delay1ms(2000)
    Delay_Until_Collision(2000, 0);


    // Move the motors backward with 30% duty cycle
    Motor_Backward(4500, 4500);


    // Make a function call to Clock_Delay1ms(2000)
    Delay_Until_Collision(2000, 0);


    // Stop the motors
    Motor_Stop();


    // Make a function call to Clock_Delay1ms(1000)
    Delay_Until_Collision(1000, 0);


    // Make the robot turn to the right with 10% duty cycle
    Motor_Right(1500, 1500);


    // Make a function call to Clock_Delay1ms(4000)
    Delay_Until_Collision(4000, 0);


    // Stop the motors
    Motor_Stop();


    // Make a function call to Clock_Delay1ms(2000)
    Delay_Until_Collision(2000, 0);


    // Set the collision_detected flag to 0
    collision_detected = 0;
    P8->OUT &= ~0xC0;
}

int main(void)
{
    // Initialize the 48 MHz Clock
    Clock_Init48MHz();

    // Initialize the built-in red LED and the RGB LEDs
    LED1_Init();
    LED2_Init();

    // Initialize the user buttons
    Buttons_Init();

    // Initialize the front and back LEDs on the chassis board
    Chassis_Board_LEDs_Init();

    // Initialize EUSCI_A0_UART
    EUSCI_A0_UART_Init_Printf();

    // Initialize the SysTick timer to generate periodic interrupts every 1 ms
    SysTick_Interrupt_Init(SYSTICK_INT_NUM_CLK_CYCLES, SYSTICK_INT_PRIORITY);

    // Initialize Timer A2 with a period of 50 Hz
    // Timer A2 will be used to drive two servos
    Timer_A2_PWM_Init(TIMER_A2_PERIOD_CONSTANT, 0, 0);

    // Initialize the bumper switches which will be used to generate external I/O-triggered in
    Bumper_Switches_Init(&Bumper_Switches_Handler);

    // Initialize the DC motors
    Motor_Init();

    // Enable the interrupts used by the SysTick and Timer A1 timers
    EnableInterrupts();

    while(1)

    {


        // Rotate to 0
        /*
        Timer_A2_Update_Duty_Cycle_1(1700);
        Timer_A2_Update_Duty_Cycle_2(1700);
        LED2_Output(RGB_LED_RED);
        Clock_Delay1ms(3000);

        // Rotate to 180
        Timer_A2_Update_Duty_Cycle_1(7000);
        Timer_A2_Update_Duty_Cycle_2(7000);
        LED2_Output(RGB_LED_BLUE);
        Clock_Delay1ms(3000);
        */

        if (collision_detected == 1)
        {
            Handle_Collision();
        }
        else
        {
            Motor_Forward(6000,6000);
        }
    }
}
