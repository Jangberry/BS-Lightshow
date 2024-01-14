#include <avr/io.h>		   // register definitions
#include <avr/pgmspace.h>  // storing data in program memory
#include <avr/interrupt.h> // interrupt vectors
#include <util/delay.h>	   // delay functions
#include <avr/sleep.h>	   // power management

#include <stdint.h>	 // C header for int types like uint8_t
#include <stdbool.h> // C header for the bool type

// Application constants
#define F_CPU 16000000UL // CPU frequency in Hz
// #define LEDS_TWO_BY_TWO	// if this variable is defined, send every pixel twice in order to double the nomber of LEDs to drive without doubling the ram usage (I wanna use 1200 but the RAM maxes out around 700)
#define NB_LED_ADDR 600
#define LED_STRIP_PORT PORTD
#define LED_STRIP_DDR DDRD
#define LED_STRIP_PIN 6
#define NB_NONADDR_STRIP 1
#define NB_LED (NB_LED_ADDR + NB_NONADDR_STRIP)
#define LED_STRIP_PIN_R 9
#define LED_STRIP_PIN_G 10
#define LED_STRIP_PIN_B 11
#define POTENTIOMETER_PIN 0
#define CHRISTMAS

// Include stuff from the library
#include "lib/iopins.h"
#include "lib/usart.h"
#include "lib/pwm.h"
#include "lib/adc.h"

/**
 * @brief Current place of the cursor in the `leds.raw` array
 */
volatile uint16_t usart_bytes_read = NB_LED * 3;
/**
 * @brief End of the window in the current transmission (index in the `leds.raw` array)
 */
volatile uint16_t usart_bytes_window_end = 0;
/**
 * @brief Number of repetitions of the current transmission
 */
volatile uint16_t usart_bytes_to_copy = 0;
/**
 * @brief Current state of the USART
 */
volatile enum {
	DISCONNECTED,
	READY,
	READING,
	COPYING,
	WAITING_FOR_START,
	WAITING_FOR_END,
	WAITING_FOR_COPY_START,
	WAITING_FOR_COPY_END,
} usart_state = DISCONNECTED;

/**
 * @brief Array containing the RGB values of the LEDs
 */
volatile union LEDS
{
	struct RGB
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} rgb[NB_LED];
	uint8_t raw[NB_LED * 3];
} leds;
/**
 * @brief Flag to indicate that the LEDs need to be updated
 */
volatile bool update_leds = true;
/**
 * @brief 16 bits 60Hz timer
 */
volatile uint16_t timer = 0;

/**
 * @brief Stroboscope frequency
 */
uint8_t stroboscope_freq = 0;
/**
 * @brief Stroboscope state
 */
bool stroboscope = true;
/**
 * @brief Last stroboscope state
 */
bool stroboscope_last = false;

// 255*(exp(-5*x/1024))-1.75*x/1024
const uint8_t PROGMEM expTable[1024] = {255, 253, 252, 251, 250, 248, 247, 246, 245, 244, 242, 241, 240, 239, 238, 236, 235, 234, 233, 232, 231, 230, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 181, 180, 179, 178, 177, 176, 175, 174, 174, 173, 172, 171, 170, 169, 169, 168, 167, 166, 165, 164, 164, 163, 162, 161, 160, 160, 159, 158, 157, 157, 156, 155, 154, 154, 153, 152, 151, 151, 150, 149, 148, 148, 147, 146, 145, 145, 144, 143, 143, 142, 141, 141, 140, 139, 138, 138, 137, 136, 136, 135, 134, 134, 133, 132, 132, 131, 131, 130, 129, 129, 128, 127, 127, 126, 125, 125, 124, 124, 123, 122, 122, 121, 121, 120, 119, 119, 118, 118, 117, 117, 116, 115, 115, 114, 114, 113, 113, 112, 111, 111, 110, 110, 109, 109, 108, 108, 107, 107, 106, 106, 105, 105, 104, 104, 103, 103, 102, 102, 101, 101, 100, 100, 99, 99, 98, 98, 97, 97, 96, 96, 95, 95, 94, 94, 93, 93, 92, 92, 91, 91, 91, 90, 90, 89, 89, 88, 88, 88, 87, 87, 86, 86, 85, 85, 85, 84, 84, 83, 83, 82, 82, 82, 81, 81, 80, 80, 80, 79, 79, 78, 78, 78, 77, 77, 77, 76, 76, 75, 75, 75, 74, 74, 74, 73, 73, 72, 72, 72, 71, 71, 71, 70, 70, 70, 69, 69, 69, 68, 68, 68, 67, 67, 67, 66, 66, 66, 65, 65, 65, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 61, 61, 60, 60, 60, 59, 59, 59, 59, 58, 58, 58, 57, 57, 57, 56, 56, 56, 56, 55, 55, 55, 55, 54, 54, 54, 53, 53, 53, 53, 52, 52, 52, 52, 51, 51, 51, 51, 50, 50, 50, 50, 49, 49, 49, 49, 48, 48, 48, 48, 47, 47, 47, 47, 46, 46, 46, 46, 46, 45, 45, 45, 45, 44, 44, 44, 44, 44, 43, 43, 43, 43, 42, 42, 42, 42, 42, 41, 41, 41, 41, 41, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 38, 38, 38, 38, 38, 37, 37, 37, 37, 37, 36, 36, 36, 36, 36, 36, 35, 35, 35, 35, 35, 34, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 27, 26, 26, 26, 26, 26, 26, 26, 25, 25, 25, 25, 25, 25, 25, 25, 24, 24, 24, 24, 24, 24, 24, 24, 23, 23, 23, 23, 23, 23, 23, 23, 22, 22, 22, 22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Timer 0 overflow handler (every 16,39ms)
ISR(TIMER0_OVF_vect)
{
	if (++timer == 0)
	{
		// 16,39ms * 65536 ≅ 131s
		usart_state = DISCONNECTED;
		adc_enable();
	}
}

// UART receive handler
ISR(USART_RX_vect)
{
	uint8_t b = usart_rx();

	timer = 0;

	switch (usart_state)
	{
	case READING:
		// Raw access to the array is faster (according to the nb of instructioons in asm (5)) than using a switch (46) or raw access to the color (43)

		leds.raw[usart_bytes_read] = b;

		usart_bytes_read++;

		if (usart_bytes_read > usart_bytes_window_end)
			usart_state = READY;

		return;
		break; // Might be redundant, but will probably be optimized out by the compiler
	case COPYING:
		for (uint16_t i = 0; i < usart_bytes_to_copy; i++)
		{
			leds.raw[usart_bytes_read + 3 * i] = b;
		}

		usart_bytes_read++;
		if (usart_bytes_read > usart_bytes_window_end)
			usart_state = READY;

		return;
		break;
	case WAITING_FOR_START:
		usart_isr_rx_enable(false); // disable RX interrupt handler to keep the second Byte from interrupting
		usart_bytes_read = ((b << 8) + usart_rx()) * 3;
		usart_state = WAITING_FOR_END;
		usart_isr_rx_enable(true); // enable RX interrupt handler
		return;
		break;
	case WAITING_FOR_END:
		// Special case when only a window of the strip us updated
		usart_isr_rx_enable(false); // disable RX interrupt handler to keep the second Byte from interrupting
		usart_bytes_window_end = usart_bytes_read + ((b << 8) + usart_rx()) * 3;
		usart_state = READING;
		usart_isr_rx_enable(true); // enable RX interrupt handler
		return;
		break;
	case WAITING_FOR_COPY_START:
		usart_isr_rx_enable(false); // disable RX interrupt handler to keep the second Byte from interrupting
		usart_bytes_read = ((b << 8) + usart_rx()) * 3;
		usart_bytes_window_end = usart_bytes_read + 3;
		usart_state = WAITING_FOR_COPY_END;
		usart_isr_rx_enable(true); // enable RX interrupt handler
		return;
		break;
	case WAITING_FOR_COPY_END:
		usart_isr_rx_enable(false); // disable RX interrupt handler to keep the second Byte from interrupting
		usart_bytes_to_copy = ((b << 8) + usart_rx());
		usart_state = COPYING;
		usart_isr_rx_enable(true); // enable RX interrupt handler
		return;
		break;
	case DISCONNECTED:
		adc_disable();
	case READY:
		switch (b)
		{
		case 0x01:
			usart_bytes_read = 0;
			usart_bytes_window_end = 3 * NB_LED;
			usart_state = READING;
			return;
			break;
		case 0x02:
			update_leds = true;
			usart_state = READY;
			return;
			break;
		case 0x03:
			// Special case when only a window of the strip us updated
			usart_state = WAITING_FOR_COPY_START;
			return;
			break;
		case 0x04:
			return;
			break;
		}
		break;
	default:
		break;
	}
}

// LED strip write function (from https://github.com/pololu/pololu-led-strip-avr/blob/master/led_strip.c)
void __attribute__((noinline)) led_strip_write(struct RGB *colors, uint16_t count)
{
	// Set the pin to be an output driving low.
	LED_STRIP_PORT &= ~(1 << LED_STRIP_PIN);
	LED_STRIP_DDR |= (1 << LED_STRIP_PIN);

	cli(); // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
	while (count--)
	{
		// Send a color to the LED strip.
		// The assembly below also increments the 'colors' pointer,
		// it will be pointing to the next color at the end of this loop.
		asm volatile(
			"ld __tmp_reg__, %a0+\n"
			"ld __tmp_reg__, %a0\n"
			"rcall send_led_strip_byte%=\n" // Send red component.
			"ld __tmp_reg__, -%a0\n"
			"rcall send_led_strip_byte%=\n" // Send green component.
			"ld __tmp_reg__, %a0+\n"
			"ld __tmp_reg__, %a0+\n"
			"ld __tmp_reg__, %a0+\n"
			"rcall send_led_strip_byte%=\n" // Send blue component.
			"rjmp led_strip_asm_end%=\n"	// Jump past the assembly subroutines.

			// send_led_strip_byte subroutine:  Sends a byte to the LED strip.
			"send_led_strip_byte%=:\n"
			"rcall send_led_strip_bit%=\n" // Send most-significant bit (bit 7).
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n" // Send least-significant bit (bit 0).
			"ret\n"

			// send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
			// high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
			// but this function always takes the same time (2 us).
			"send_led_strip_bit%=:\n"
#if F_CPU == 8000000
			"rol __tmp_reg__\n" // Rotate left through carry.
#endif
			"sbi %2, %3\n" // Drive the line high.

#if F_CPU != 8000000
			"rol __tmp_reg__\n" // Rotate left through carry.
#endif

#if F_CPU == 16000000
			"nop\n"
			"nop\n"
#elif F_CPU == 20000000
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
#elif F_CPU != 8000000
#error "Unsupported F_CPU"
#endif

			"brcs .+2\n"
			"cbi %2, %3\n" // If the bit to send is 0, drive the line low now.

#if F_CPU == 8000000
			"nop\n"
			"nop\n"
#elif F_CPU == 16000000
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
#elif F_CPU == 20000000
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
			"nop\n"
#endif

			"brcc .+2\n"
			"cbi %2, %3\n" // If the bit to send is 1, drive the line low now.

			"ret\n"
			"led_strip_asm_end%=: "
			: "=b"(colors)
			: "0"(colors),						 // %a0 points to the next color to display
			  "I"(_SFR_IO_ADDR(LED_STRIP_PORT)), // %2 is the port register (e.g. PORTC)
			  "I"(LED_STRIP_PIN)				 // %3 is the pin number (0-8)
		);

		// Uncomment the line below to temporarily enable interrupts between each color.
		// sei(); asm volatile("nop\n"); cli();
	}
	sei(); // Re-enable interrupts now that we are done.
		   // _delay_us(1); // Send the reset signal.
}

void main()
{
	usart_init(BAUD_1M);
	usart_isr_rx_enable(true); // enable RX interrupt handler
	usart_set_2x(true);		   // double speed

	// init leds
	LED_STRIP_DDR |= (1 << LED_STRIP_PIN);

	// init pwm
	as_output(LED_STRIP_PIN_R);
	as_output(LED_STRIP_PIN_G);
	as_output(LED_STRIP_PIN_B);
	pwm_init();

	// init timer 0
	TCCR0B |= (1 << CS00) | (1 << CS02); // prescaler 1024
	TIMSK0 |= (1 << TOIE0);				 // enable overflow interrupt
	// overflow every 16M/1024/256 = 61,03Hz = 16,39ms

	// init adc
	adc_init(ADC_PRESC_128);
	adc_enable();

	// globally enable interrupts (for the USART_RX handler)
	sei();

	while (1)
	{
		if (usart_state == DISCONNECTED)
		{
			stroboscope_freq = pgm_read_byte(&expTable[adc_read_10bit()]);
			adc_start_conversion(POTENTIOMETER_PIN);

			if (stroboscope_freq > 2)
			{
				stroboscope = (timer % stroboscope_freq) < (stroboscope_freq / 2);
			}
			else
			{
				stroboscope = true;
#ifdef CHRISTMAS
				uint16_t train = (timer >> 2) % NB_LED;
				for (uint16_t i = 0; i < NB_LED; i++)
				{
					if (i >= train && i <= train + 11)
					{
						// Black for the place between the reindeers
						if (i == train + 11 || i == train + 9 || i == train + 7 || i == train + 5 || i == train)
						{
							leds.rgb[i].r = 0;
							leds.rgb[i].g = 0;
							leds.rgb[i].b = 0;
						}
						// Brown for the reindeers
						else if (i == train + 10 || i == train + 8 || i == train + 6)
						{
							leds.rgb[i].r = 200;
							leds.rgb[i].g = 128;
							leds.rgb[i].b = 20;
						}
						// White for the sled
						else if (i == train + 4 || i == train + 1)
						{
							leds.rgb[i].r = 255;
							leds.rgb[i].g = 255;
							leds.rgb[i].b = 255;
						}
						// Red for santa claus
						else if (i == train + 3 || i == train + 2)
						{
							leds.rgb[i].r = 255;
							leds.rgb[i].g = 0;
							leds.rgb[i].b = 0;
						}
					}
					else
					{
						leds.rgb[i].r = (i % 2 == 0) ^ ((timer & 0x20) > 0x10) ? 255 : 0;
						leds.rgb[i].g = (i % 2 == 1) ^ ((timer & 0x20) > 0x10) ? 255 : 0;
						leds.rgb[i].b = 0;
					}
				}
				update_leds = true;
#endif
			}

			if (stroboscope_last != stroboscope)
			{
				uint8_t brightness = stroboscope ? 200 : 0;

				for (uint16_t i = 0; i < NB_LED; i++)
				{
					leds.rgb[i].r = brightness;
					leds.rgb[i].g = brightness;
					leds.rgb[i].b = brightness;
				}

				stroboscope_last = stroboscope;
				update_leds = true;
			}
		}

		if (update_leds)
		{
			led_strip_write(leds.rgb, NB_LED_ADDR);

#if LED_STRIP_PIN_R != 9 || (LED_STRIP_PIN_G != 10) || (LED_STRIP_PIN_B != 11)
#warning "LED_STRIP_PIN_R, LED_STRIP_PIN_G and LED_STRIP_PIN_B must be 9, 10 and 11 respectively; Or you need to change this accordingly"
#endif

			OCR1A = leds.rgb[NB_LED_ADDR].r;
			OCR1B = leds.rgb[NB_LED_ADDR].g;
			OCR2A = leds.rgb[NB_LED_ADDR].b;

			update_leds = false;
		}

		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_cpu();
	}
}
