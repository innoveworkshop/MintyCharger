/**
 * interface.c
 * Handles all the user interface stuff.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "interface.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "vreg.h"

// Private enumerators.
typedef enum { NIMH_72V, LTION_74V, NIMH_84V, NIMH_96V } battery_t;
typedef enum { RATE_TRICKLE, RATE_15MA, RATE_50MA, RATE_75MA, RATE_100MA } rate_t;
typedef enum { MODE_CHARGE, MODE_DISCHARGE, MODE_REFRESH } mode_t;

// Private variables.
battery_t selectedBattery = NIMH_72V;
rate_t    selectedRate    = RATE_15MA;
mode_t    selectedMode    = MODE_CHARGE;

/**
 * Light up the correct LEDs depending on what's configured.
 */
void DisplayCurrentConfiguration(void) {
	
}

/**
 * Selects the next battery voltage LED.
 */
void SelectNextVoltage(void) {
	// Change configuration.
	if (selectedBattery < NIMH_96V) {
		selectedBattery++;
	} else {
		selectedBattery = NIMH_72V;
	}
	
	// Set the regulator voltage.
	switch (selectedBattery) {
		case NIMH_72V:
			SetTargetVoltage(9.0f);
			break;
		case LTION_74V:
			SetTargetVoltage(8.4f);
			break;
		case NIMH_84V:
			SetTargetVoltage(10.5f);
			break;
		case NIMH_96V:
			SetTargetVoltage(12.0f);
			break;
	}
	
	// Show change on board.
	DisplayCurrentConfiguration();
}

/**
 * Selects the next charge/discharge rate LED.
 */
void SelectNextRate(void) {
	// Change configuration.
	if (selectedRate < RATE_100MA) {
		selectedRate++;
	} else {
		selectedRate = RATE_15MA;
	}
	
	// Set the regulator current.
	switch (selectedRate) {
		case RATE_TRICKLE:
			SetTargetCurrent(0.0076f);
			break;
		case RATE_15MA:
			SetTargetCurrent(0.015f);
			break;
		case RATE_50MA:
			SetTargetCurrent(0.05f);
			break;
		case RATE_75MA:
			SetTargetCurrent(0.075f);
			break;
		case RATE_100MA:
			SetTargetCurrent(0.1f);
			break;
	}
	
	// Show change on board.
	DisplayCurrentConfiguration();
}

/**
 * Selects the next charge mode LED.
 */
void SelectNextMode(void) {
	// Change configuration.
	if (selectedMode < MODE_REFRESH) {
		selectedMode++;
	} else {
		selectedMode = MODE_CHARGE;
	}
	
	// Show change on board.
	DisplayCurrentConfiguration();
}
