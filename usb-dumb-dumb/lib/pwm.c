#include "pwm.h"

void pwm_init(){
    #if LED_STRIP_PIN_R != 9 || (LED_STRIP_PIN_G != 10) || (LED_STRIP_PIN_B != 11)
        #warning "LED_STRIP_PIN_R, LED_STRIP_PIN_G and LED_STRIP_PIN_B must be 9, 10 and 11 respectively; Or you need to change this file accordingly"
    #endif
    // Set timer 1 prescaler to 1
    TCCR1B |= (1 << CS10);
    // Set timer 2 prescaler to 1
    TCCR2B |= (1 << CS20);
    
    // Set timer 1 to Fast PWM mode
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);
    // Set timer 2 to Fast PWM mode
    TCCR2A |= (1 << WGM21) | (1 << WGM20);
    
    // Set timer 1 to non-inverting mode
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
    // Set timer 2 to non-inverting mode
    TCCR2A |= (1 << COM2A1);

    // Set timer 1 max value (timer1 is 16 bits so we need to set it to acts as an 8 bit one to match timer 2)
    ICR1 = 255;
    
    // Set timer 1 output compare values
    OCR1A = 0;
    OCR1B = 0;

    // Set timer 2 output compare value
    OCR2A = 0;
}

void pwm_set(uint8_t value, uint8_t pin){
    switch(pin){
        case 9:
            OCR1A = value;
            break;
        case 10:
            OCR1B = value;
            break;
        case 11:
            OCR2A = value;
            break;
        default:
            break;
    }
}
