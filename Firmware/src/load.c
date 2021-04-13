/**
 * load.c
 * Electronic load to discharge the battery.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "eeprom.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "vreg.h"

// Private variables.
bool enabled = false;

/**
 * Enables the electronic load.
 */
void EnableLoad(void) {
	// Disable the voltage regulator.
	DisableRegulator();
	
	// Reset DAC to 0 and set the enabled flag.
	//SetLoad(0);
	enabled = true;
}

/**
 * Disables the electronic load.
 */
void DisableLoad(void) {
	// Reset DAC to 0 and set the enabled flag.
	//SetLoad(0);
	enabled = false;
}