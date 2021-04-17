/**
 * version.c
 * Shows the user the current version of the firmware they are running.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "version.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"

// Check if the firmware version was defined.
#ifndef FIRMWARE_VERSION
	#error "FIRMWARE_VERSION wasn't defined!"
#endif

/**
 * Displays the firmware information while the user has the select button held
 * down.
 */
inline void DisplayFirmwareInformation(void) {
	while ((PORTA & BTN_SELECT) == 0) {
		LATC = (uint8_t)~((FIRMWARE_VERSION & 0b1111) << 2);
		__delay_ms(100);
	}
}