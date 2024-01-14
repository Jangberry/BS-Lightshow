#pragma once

#include "iopins.h"

/**
 * @brief Initialize PWM for pins 9, 10 and 11
 * Uses Timer 1 and 2
 */
void pwm_init();

/**
 * @brief Set PWM value
 * @param value 8 bit value to set
 * @param pin Pin to set (9, 10 or 11)
 */
void pwm_set(uint8_t value, uint8_t pin);