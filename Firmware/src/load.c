/**
 * load.c
 * Electronic load to discharge the battery.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "load.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "vreg.h"
#include "interface.h"

// Some definitions.
#define LOAD_VREF_VOLTAGE 1.024f  // V

// Private variables.
bool loadEnabled    = false;
uint8_t loadCurrent = 0;

// Private methods.
extern inline uint8_t __attribute__((always_inline)) GetLoadCurrentValue(void);
extern inline void __attribute__((always_inline)) SetLoadCurrentValue(const uint8_t current);

/**
 * Enables the electronic load.
 */
void EnableLoad(void) {
	// Reset DAC to 0 and set the enabled flag.
	DACCON1bits.DAC1R = loadCurrent;
	loadEnabled = true;
}

/**
 * Disables the electronic load.
 */
void DisableLoad(void) {
	// Reset DAC to 0 and set the enabled flag.
	DACCON1bits.DAC1R = 0;
	loadEnabled = false;
}

/**
 * Determine the current state of the battery and disable the electronic load if
 * it's time to do so.
 */
void DetectLoadCutoff(void) {
	if (IsLoadEnabled()) {
		if (IsAtCutoffVoltage())
			DisableLoad();
	}
}

/**
 * Checks if the electronic load is active.
 * 
 * @return TRUE if the load is currently active.
 */
inline bool IsLoadEnabled(void) {
	return loadEnabled;
}

/**
 * Detects if the battery has reached the discharge cutoff voltage.
 * 
 * @return TRUE if the battery has finished discharging.
 */
bool IsAtCutoffVoltage(void) {
	// Handle the lithium battery case.
	if (IsLithiumBattery())
		return GetCellVoltage() < 3.0f;
	
	// Detect the cutoff for normal NiMH batteries.
	return GetCellVoltage() < 0.95f;
}

/**
 * Gets the set load current.
 * 
 * @return Load current.
 */
float GetLoadCurrent(void) {
	return (((float)GetLoadCurrentValue() * LOAD_VREF_VOLTAGE) / 32.0f) / 10.0f;
}

/**
 * Sets the regulator target current.
 * 
 * @param current Target current.
 */
void SetLoadCurrent(const float current) {
	SetLoadCurrentValue((uint8_t)((float)(320 * current) / LOAD_VREF_VOLTAGE));
}

/**
 * Gets the load current value that's set in the DAC. (This is 10x higher than
 * the actual current, since we have a 10ohm shunt resistor)
 * 
 * @return Load current (10x) value.
 */
inline uint8_t GetLoadCurrentValue(void) {
	return loadCurrent;
}

/**
 * Sets the load current value. (This is 10x higher than the actual current,
 * since we have a 10ohm shunt resistor)
 * 
 * @param current Load current value (10x higher).
 */
inline void SetLoadCurrentValue(const uint8_t current) {
	loadCurrent = current & 0b11111;
}