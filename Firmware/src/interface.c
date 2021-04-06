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

// Private definitions.
#define SHIFT_CLOCK_DELAY 10 // us
#define LIGHT_SHOW_DELAY  20 // ms

// Private enumerators.
typedef enum { NIMH_72V, LTION_74V, NIMH_84V, NIMH_96V } battery_t;
typedef enum { RATE_15MA, RATE_50MA, RATE_75MA, RATE_100MA, RATE_TRICKLE } rate_t;
typedef enum { MODE_CHARGE, MODE_DISCHARGE, MODE_REFRESH } mode_t;
typedef enum { SEL_RUNNING, SEL_BATTERY, SEL_MODE, SEL_RATE } selection_t;

// Private variables.
battery_t   selectedBattery  = NIMH_72V;
rate_t      selectedRate     = RATE_15MA;
mode_t      selectedMode     = MODE_CHARGE;
selection_t currentSelection = SEL_RUNNING;
uint16_t    configLights     = 0;

// Private methods.
void ShiftData(const uint16_t data);
inline uint16_t __attribute__((always_inline)) SelectedBatteryLED(void);
inline uint16_t __attribute__((always_inline)) SelectedRateLED(void);
inline uint16_t __attribute__((always_inline)) SelectedModeLED(void);
void DisplayCurrentConfiguration(void);
void SelectNextVoltage(void);
void SelectNextRate(void);
void SelectNextMode(void);

/**
 * Initializes the user interface by putting on a little show.
 */
void InitializeUI(void) {
	// Turn all the lights off for the show.
	LATC |= CHG_LED0 + CHG_LED1 + CHG_LED2 + CHG_LED3;
	ShiftData(0);
	
	// Chase the charging indicators.
	for (uint8_t i = 0; i < 4; i++) {
		LATC &= ~(CHG_LED0 << i);
		__delay_ms(LIGHT_SHOW_DELAY);
		LATC |= CHG_LED0 + CHG_LED1 + CHG_LED2 + CHG_LED3;
	}
	
	// Chase all the configuration LEDs.
	for (uint8_t i = 0; i < 11; i++) {
		ShiftData(0b1000000000000000 >> i);
		__delay_ms(LIGHT_SHOW_DELAY);
	}
	
	// Enable the flashing timer.
	T6CONbits.TMR6ON = 1;
	
	// Display the default startup configuration.
	DisplayCurrentConfiguration();
}

/**
 * Indicates to the user that he or she is editing a configuration by flashing
 * its corresponding LED.
 */
void FlashCurrentEditableConfiguration(void) {
	// Toggle the current selection light.
	switch (currentSelection) {
		case SEL_BATTERY:
			configLights ^= SelectedBatteryLED();
			break;
		case SEL_MODE:
			configLights ^= SelectedModeLED();
			break;
		case SEL_RATE:
			configLights ^= SelectedRateLED();
			break;
		case SEL_RUNNING:
			break;
	}
	
	// Shift the light data out.
	ShiftData(configLights);
}

/**
 * Light up the correct LEDs depending on what's currently configured.
 */
void DisplayCurrentConfiguration(void) {
	// Reset the lights.
	configLights = 0;

	// Configure each light that should light up.
	configLights += SelectedBatteryLED();
	configLights += SelectedRateLED();
	configLights += SelectedModeLED();
	
	// Shift the light data out.
	ShiftData(configLights);
}

/**
 * Selects the next option of the current user selection.
 */
void SelectNextOption(void) {
	switch (currentSelection) {
		case SEL_BATTERY:
			SelectNextVoltage();
			break;
		case SEL_MODE:
			SelectNextMode();
			break;
		case SEL_RATE:
			SelectNextRate();
			break;
		case SEL_RUNNING:
			break;
	}
}

/**
 * Goes into the next selection stage.
 */
void NextConfigurationSelection(void) {
	// Change current user selection.
	if (currentSelection < SEL_RATE) {
		currentSelection++;
	} else {
		currentSelection = SEL_RUNNING;
	}
	
	// Show change on board.
	DisplayCurrentConfiguration();
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

/**
 * Gets the selected battery configuration LED to shift out.
 * 
 * @return Selected battery configuration LED position.
 */
inline uint16_t SelectedBatteryLED(void) {
	return 1 << (selectedBattery - 3 + 15);
}

/**
 * Gets the selected rate configuration LED to shift out.
 * 
 * @return Selected rate configuration LED position.
 */
inline uint16_t SelectedRateLED(void) {
	// Ignore the trickle rate.
	if (selectedRate == RATE_TRICKLE)
		return 0;
	
	return 1 << (selectedRate - 3 + 11);
}

/**
 * Gets the selected mode configuration LED to shift out.
 * 
 * @return Selected mode configuration LED position.
 */
inline uint16_t SelectedModeLED(void) {
	return 1 << (selectedMode - 2 + 7);
}

/**
 * Sends a 16-bit value to the dual shift registers we have dealing with most of
 * the lights.
 * 
 * @param data 16-bit data to be sent to the shift register.
 */
void ShiftData(const uint16_t data) {
	// Unlatch.
	LATA &= ~(SR_DATA + SR_CLOCK + SR_LATCH);
	
	// Go through each bit sending them LSB-first.
	for (uint8_t i = 0; i < 16; i++) {
		if (data & (1 << i)) {
			// 1
			LATA |= SR_DATA;
			__delay_us(SHIFT_CLOCK_DELAY);
			LATA |= SR_CLOCK;
			__delay_us(SHIFT_CLOCK_DELAY);
		} else {
			// 0
			LATA &= ~(SR_DATA + SR_CLOCK);
			__delay_us(SHIFT_CLOCK_DELAY);
			LATA |= SR_CLOCK;
			__delay_us(SHIFT_CLOCK_DELAY);
		}
		
		// Make sure all lines are LOW.
		LATA &= ~(SR_DATA + SR_CLOCK);
	}
	
	// Release the latch.
	LATA |= SR_LATCH;
}
