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

/**
 * Application main entry point.
 */
void main(void) {
	while (true) {
	}

	return;
}
