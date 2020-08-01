/**
 * MintyCharger
 * A complete 9V (9.6V and 8.4V) NiMH charger that fits neatly inside an Altoids
 * tin.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

//                           16F18325
//                         +----------+
//                        -|RA0    RC0|- Voltage Sense - ANC0
//                        -|RA1    RC1|- Current Sense - ANC1
//                        -|RA2    RC2|- Charge LED 0
// Select Switch          -|RA3    RC3|- Charge LED 1
// Discharge Enable       -|RA4    RC4|- Charge LED 2
// Boost DC/DC PWM - PWM1 -|RA5    RC5|- Charge LED 3
//                         +----------+

#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"

// Function prototypes.
void SetupPins();

/**
 * Application main entry point.
 */
void main(void) {
	uint8_t pins = CHG_LED0;
	
	// Initialize pins.
	SetupPins();
	
	while (true) {
		LATC = ~pins;
		
		if (pins < CHG_LED3) {
			pins <<= 1;
		} else {
			pins = CHG_LED0;
		}
		
		__delay_ms(500);
	}

	return;
}

/**
 * Sets up all the pins for the application.
 */
void SetupPins() {
	// Setup digital inputs.
	TRISA = 0b111 + BTN_SELECT;
	TRISC = VSENSE + ISENSE;
	
	// Setup pull-ups.
	WPUC = 0;
	
	// Setup open-drains.
	ODCONC = CHG_LED0 + CHG_LED1 + CHG_LED2 + CHG_LED3;
	
	// Set all the outputs to LOW.
	LATA = 0;
	LATC = 0;
}