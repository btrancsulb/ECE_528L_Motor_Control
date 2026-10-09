/**
 * @file Bumper_Switches.c
 * @brief Source code for the Bumper_Switches driver.
 *
 * This file contains the function definitions for the Bumper_Switches driver.
 * It is used to demonstrate external I/O-triggered interrupts.
 * The driver interfaces with the following:
 *  - Pololu Left Bumper Switches (https://www.pololu.com/product/3673)
 *  - Pololu Right Bumper Switches (https://www.pololu.com/product/3674)
 *
 * To verify the pinout used for the Pololu Bumper Switches, refer to the MSP432P401R SimpleLink Microcontroller LaunchPad Development Kit User's Guide
 * Link: https://docs.rs-online.com/3934/A700000006811369.pdf
 *
 * The following pins are used when the Bumper Switches are connected to the TI MSP432 LaunchPad:
 *  - BUMP_0                    <-->    MSP432 LaunchPad Pin P4.0
 *  - BUMP_1                    <-->    MSP432 LaunchPad Pin P4.2
 *  - BUMP_2                    <-->    MSP432 LaunchPad Pin P4.3
 *  - Right Bumper Switch GND   <-->    MSP432 LaunchPad GND
 *  - BUMP_3                    <-->    MSP432 LaunchPad Pin P4.5
 *  - BUMP_4                    <-->    MSP432 LaunchPad Pin P4.6
 *  - BUMP_5                    <-->    MSP432 LaunchPad Pin P4.7
 *  - Left Bumper Switch GND    <-->    MSP432 LaunchPad GND
 *
 * @note The Bumper Switches operate in an active low configuration.
 * When the switches are pressed, they connect to GND.
 *
 * @author Aaron Nanas
 *
 */

#include "../inc/Bumper_Switches.h"

#define BUMPER_PIN_MASK 0xED

void Bumper_Switches_Init(void(*task)(uint8_t))
{
    // Store the user-defined task function for use during interrupt handling
    Bumper_Task = task;

    // Configure the following pins as GPIO pins: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0
    // and enable pull up resistors
    // by clearing the corresponding bits in the SEL0 and SEL1 registers
    P4->SEL0 &= ~(0xED);
    P4->SEL1 &= ~(0xED);



    // Set the direction of the following pins as input: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0
    // by clearing the corresponding bits in the DIR register
    // Configure the following pins to use falling-edge interrupt triggers by
    // setting the corresponding bits in the IES register: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0.
    P4->DIR &= ~(0xED);
    P4->IES |= 0xED;


    // Enable pull-up resistors on the following pins: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0
    // by setting the corresponding bits in the REN register
    P4->REN |= 0xED;


    // Ensure that the pins are pulled up: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0
    // by setting the corresponding bits in the OUT register
    P4->OUT |= 0xED;


    // Interrupt Edge Select: High-to-Low Transition
    // Configure the pins to use falling edge event triggers: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0
    // by setting the corresponding bits in the IES register
    P4->IES |= 0xED;


    // Clear any existing interrupt flags on the following pins: P4.7, P4.6, P4.5, P4.3, P4.2, and P4.0
    // by clearing the corresponding bits in the IFG register
    P4->IFG &= ~(0xED);


    // Enable interrupts on the following pins: P4.7 - P4.5, P4.3, P4.2, and P4.0
    // by setting the corresponding bits in the IE register
    P4->IE |= 0xED;


    // Set the priority level of the interrupts (IRQ 38) to 0 (section 2.4.3.20)
    NVIC->IP[9] = (NVIC->IP[9] & 0xFF0FFFFF);

    // Enable Interrupt 38 in NVIC (section 2.4.3.2)
    // Bit 6 corresponds to IRQ 38
    NVIC->ISER[1] = 0x00000040;
}

uint8_t Bumper_Read(void)
{
    // Declare a local variable to store the input register value
    // Then, read the input register P4->IN and invert its value using the bitwise NOT operator (~).
    // This is done to account for the negative logic behavior of the bumper switches
    uint32_t bumper_state = ~P4->IN;


    // Use bitwise operations to extract the relevant bits representing the switch states.
    // - ((bumper_state & 0xE0) >> 2): Extract bits 7, 6, and 5, and right-shift them by 2 to align them to bits 5, 4, and 3.
    // - ((bumper_state & 0x0C) >> 1): Extract bits 3 and 2, and right-shift them by 1 to align them to bits 2 and 1.
    // - (bumper_state & 0x01): Extract bit 0.
    // Then, combine the extracted bits using the bitwise OR operator and return the bumper state
    return (((bumper_state & 0xE0) >> 2) | ((bumper_state & 0x0C) >> 1) | (bumper_state & 0x01));
}

/**
 * @brief Interrupt handler for PORT4 (P4) events.
 *
 * This function is an interrupt service routine (ISR) for PORT4 (P4) of the TI MSP432 LaunchPad.
 * It is triggered on a falling edge event on any of the switches connected to P4 (BUMP_0 to BUMP_5).
 * The function clears the interrupt flags, confirms the switch is still pressed, and then
 * calls the user-defined Bumper_Task with the packed bumper state.
 *
 * @return None
 */
void PORT4_IRQHandler(void)
{
    uint8_t triggered_pins = (uint8_t)(P4->IFG & P4->IE & BUMPER_PIN_MASK);
    uint8_t bumper_state;
    uint8_t pending_state;

    P4->IFG &= ~BUMPER_PIN_MASK;

    bumper_state = Bumper_Read();
    pending_state = (uint8_t)(((triggered_pins & 0xE0) >> 2) |
                              ((triggered_pins & 0x0C) >> 1) |
                              (triggered_pins & 0x01));

    if ((bumper_state & pending_state) != 0)
    {
        (*Bumper_Task)((uint8_t)(bumper_state & pending_state));
    }
}
