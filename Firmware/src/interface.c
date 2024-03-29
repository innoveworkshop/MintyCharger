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
#include "adc.h"
#include "vreg.h"
#include "load.h"
#include "refresh.h"
#include "eeprom.h"

// Private definitions.
#define SHIFT_CLOCK_DELAY 10 // us
#define LIGHT_SHOW_DELAY  20 // ms

// Private enumerators.

typedef enum {
	SEL_RUNNING, SEL_BATTERY, SEL_MODE, SEL_RATE
} selection_t;

// Private variables.
battery_t selectedBattery    = NIMH_72V;
rate_t selectedRate          = RATE_15MA;
mode_t selectedMode          = MODE_CHARGE;
selection_t currentSelection = SEL_RUNNING;
uint16_t configLights        = 0;

// Private methods.
void ShiftData(const uint16_t data);
inline uint16_t __attribute__((always_inline)) SelectedBatteryLED(void);
inline uint16_t __attribute__((always_inline)) SelectedRateLED(void);
inline uint16_t __attribute__((always_inline)) SelectedModeLED(const mode_t mode);
void LoadSettings(void);
void DisplayCurrentConfiguration(void);
void CommitConfiguration(const bool save_settings);
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

	// Chase all the configuration LEDs.
	for (uint8_t i = 0; i < 11; i++) {
		ShiftData(1 << i);
		__delay_ms(LIGHT_SHOW_DELAY);
	}

	// Chase the charging indicators.
	for (uint8_t i = 0; i < 4; i++) {
		LATC &= ~(CHG_LED0 << i);
		__delay_ms(LIGHT_SHOW_DELAY);
		LATC |= CHG_LED0 + CHG_LED1 + CHG_LED2 + CHG_LED3;
	}

	// Load settings from the EEPROM.
	LoadSettings();

	// Commit and display the default startup configuration.
	CommitConfiguration(false);
	ShiftData(0);
	DisplayCurrentConfiguration();
}

/**
 * Load settings from the EEPROM.
 */
void LoadSettings(void) {
	selectedMode = GetChargerModeSetting();
	selectedBattery = GetBatteryTypeSetting();
	selectedRate = GetChargeRateSetting();
}

/**
 * Commits the current configuration to the voltage regulator.
 * 
 * @param save_settings Save the configuration to the EEPROM?
 */
void CommitConfiguration(const bool save_settings) {
	// Disable everything for safety.
	DisableLoad();
	ClearFinishedCharging();
	DisableRegulator();

	// Set current.
	float current = 0;
	switch (GetSelectedCurrent()) {
		case RATE_TRICKLE:
			current = 0.0076f;
			break;
		case RATE_15MA:
			current = 0.015f;
			break;
		case RATE_50MA:
			current = 0.05f;
			break;
		case RATE_75MA:
			current = 0.075f;
			break;
		case RATE_100MA:
			current = 0.1;
			break;
	}
	SetTargetCurrent(current);
	SetLoadCurrent(current);
	if (save_settings)
		SaveChargeRateSetting(GetSelectedCurrent());

	// Set voltage.
	float voltage = 0;
	switch (GetSelectedBattery()) {
		case NIMH_72V:
			voltage = 8.8f;
			break;
		case LTION_74V:
			voltage = 8.2f;
			break;
		case NIMH_84V:
			voltage = 10.3f;
			break;
		case NIMH_96V:
			voltage = 11.8f;
			break;
	}
	SetTargetVoltage(voltage);
	if (save_settings)
		SaveBatteryTypeSetting(GetSelectedBattery());

	// Execute the currently selected task.
	if (save_settings) {
		HandleSingleButtonClick();
		SaveChargerModeSetting(GetSelectedMode());
	}
}

/**
 * Handle a single click of the selection button.
 */
void HandleSingleButtonClick(void) {
	// Check if we are in running more or editing configurations.
	if (currentSelection == SEL_RUNNING) {
		switch (selectedMode) {
			case MODE_CHARGE:
				// Start charging.
				DisableLoad();
				ClearFinishedCharging();
				break;
			case MODE_DISCHARGE:
				// Discharge the battery.
				DisableRegulator();
				EnableLoad();
				break;
			case MODE_REFRESH:
				// Refresh the battery.
				StopRefreshCycle();
				NextRefreshCyclePart();
				break;
		}
	} else {
		// Select the next option of the current selection.
		SelectNextOption();
	}
}

/**
 * Indicates to the user that he or she is editing a configuration by flashing
 * its corresponding LED.
 */
void FlashCurrentEditableConfiguration(void) {
	// If we are editing the configurations, make sure everything is disabled.
	if (currentSelection != SEL_RUNNING) {
		DisableRegulator();
		DisableLoad();
	}

	// Toggle the current selection light.
	switch (currentSelection) {
		case SEL_BATTERY:
			configLights ^= SelectedBatteryLED();
			break;
		case SEL_MODE:
			configLights ^= SelectedModeLED(selectedMode);
			break;
		case SEL_RATE:
			configLights ^= SelectedRateLED();
			break;
		case SEL_RUNNING:
			// Handle battery refresh.
			if (selectedMode == MODE_REFRESH) {
				switch (GetRefreshCycle()) {
					case REFRESH_DISCHARGING:
						ShiftData(configLights | SelectedModeLED(MODE_DISCHARGE));
						return;
					case REFRESH_CHARGING:
						ShiftData(configLights | SelectedModeLED(MODE_CHARGE));
						return;
					case REFRESH_STOPPED:
						// Do nothing.
						break;
				}
			}
			break;
	}

	// Shift the light data out.
	ShiftData(configLights);
}

/**
 * Light up the correct LEDs depending on what's currently configured.
 */
void DisplayCurrentConfiguration(void) {
	// Configure each light that should light up.
	configLights = SelectedBatteryLED() | SelectedRateLED() | SelectedModeLED(selectedMode);

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
			// Do nothing.
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
		// Go into running mode and commit configuration.
		currentSelection = SEL_RUNNING;
		CommitConfiguration(true);
	}

	// Show change on board.
	DisplayCurrentConfiguration();
}

/**
 * Selects the next battery voltage LED.
 */
void SelectNextVoltage(void) {
	// Change configuration.
	if (GetSelectedBattery() < NIMH_96V) {
		selectedBattery++;
	} else {
		selectedBattery = NIMH_72V;
	}

	// Show change on board.
	DisplayCurrentConfiguration();
}

/**
 * Selects the next charge/discharge rate LED.
 */
void SelectNextRate(void) {
	// Change configuration.
	if (GetSelectedCurrent() < RATE_100MA) {
		selectedRate++;
	} else {
		selectedRate = RATE_15MA;
	}

	// Show change on board.
	DisplayCurrentConfiguration();
}

/**
 * Selects the next charge mode LED.
 */
void SelectNextMode(void) {
	// Change configuration.
	if (GetSelectedMode() < MODE_REFRESH) {
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
	return 1 << selectedBattery;
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

	return 1 << (selectedRate + 4);
}

/**
 * Gets the selected mode configuration LED to shift out.
 * 
 * @return Selected mode configuration LED position.
 */
inline uint16_t SelectedModeLED(const mode_t mode) {
	return 1 << (mode + 8);
}

/**
 * Sends a 16-bit value to the dual shift registers we have dealing with most of
 * the lights. Keep in mind that the LEDs are LSB first, so bit 0 corresponds to
 * the first LED.
 * 
 * @param data 16-bit data to be sent to the shift register.
 */
void ShiftData(const uint16_t data) {
	// Unlatch.
	LATA &= ~(SR_DATA + SR_CLOCK + SR_LATCH);

	// Go through each bit sending them MSB-first.
	for (int8_t i = 15; i >= 0; i--) {
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

/**
 * Gets the currently selected battery type.
 * 
 * @return Selected battery type.
 */
inline battery_t GetSelectedBattery(void) {
	return selectedBattery;
}

/**
 * Gets the currently selected current for charging and discharging.
 * 
 * @return Selected current for charging and discharging.
 */
inline rate_t GetSelectedCurrent(void) {
	return selectedRate;
}

/**
 * Gets the currently selected charger mode.
 * 
 * @return  Selected mode of operation.
 */
inline mode_t GetSelectedMode(void) {
	return selectedMode;
}

/**
 * Checks if the currently selected battery is of the lithium kind.
 * 
 * @return Is the battery of lithium?
 */
bool IsLithiumBattery(void) {
	return selectedBattery == LTION_74V;
}

/**
 * Checks if the user is currently selecting a configuration.
 * 
 * @return Is the user selecting a configuration?
 */
bool IsSelectingConfiguration(void) {
	return currentSelection != SEL_RUNNING;
}