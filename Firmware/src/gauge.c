/* 
 * gauge.c
 * Shows the user the current state of charge of the battery.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "gauge.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "interface.h"
#include "adc.h"
#include "vreg.h"
#include "load.h"

// Private variables.
uint8_t blinkState = 0;

// Private methods.
inline void __attribute__((always_inline)) BlinkChargingState(uint8_t *gauge);

/**
 * Updates the gauge display.
 */
void DisplayBatteryGauge(void) {
	uint8_t gauge = 0b0000;
	float voltage = GetCellVoltage();

	// Change the gauge steps depending on what's currently happening.
	if (IsLoadEnabled()) {
		// Gauge in discharge mode.
		if (IsLithiumBattery()) {
			if (voltage > 4.0f) {
				// 75%
				gauge = 0b0111;
			} else if (voltage > 3.8f) {
				// 50%
				gauge = 0b0011;
			} else if (voltage > 3.6f) {
				// 25%
				gauge = 0b0001;
			}
		} else {
			if (voltage > 1.4f) {
				// 75%
				gauge = 0b0111;
			} else if (voltage > 1.3f) {
				// 50%
				gauge = 0b0011;
			} else if (voltage > 1.2f) {
				// 25%
				gauge = 0b0001;
			}
		}
	} else if (IsBatteryCharging()) {
		// Gauge in charge mode.
		if (IsLithiumBattery()) {
			if (IsFinishedCharging()) {
				// 100%
				gauge = 0b1111;
			} else if (voltage > 4.00f) {
				// 75%
				gauge = 0b0111;
			} else if (voltage > 3.85f) {
				// 50%
				gauge = 0b0011;
			} else if (voltage > 3.75f) {
				// 25%
				gauge = 0b0001;
			}
		} else {
			if (IsFinishedCharging()) {
				// 100%
				gauge = 0b1111;
			} else if (voltage > 1.42f) {
				// 75%
				gauge = 0b0111;
			} else if (voltage > 1.35f) {
				// 50%
				gauge = 0b0011;
			} else if (voltage > 1.20f) {
				// 25%
				gauge = 0b0001;
			}
		}
	} else {
		// Gauge when nothing is happening.
		if (IsLithiumBattery()) {
			if (voltage > 4.0f) {
				// 100%
				gauge = 0b1111;
			} else if (voltage > 3.7f) {
				// 75%
				gauge = 0b0111;
			} else if (voltage > 3.5f) {
				// 50%
				gauge = 0b0011;
			} else if (voltage > 3.2f) {
				// 25%
				gauge = 0b0001;
			}
		} else {
			if (voltage > 1.4f) {
				// 100%
				gauge = 0b1111;
			} else if (voltage > 1.3f) {
				// 75%
				gauge = 0b0111;
			} else if (voltage > 1.2f) {
				// 50%
				gauge = 0b0011;
			} else if (voltage > 1.1f) {
				// 25%
				gauge = 0b0001;
			}
		}
	}

	// Blink them lights.
	if (IsBatteryCharging() || IsLoadEnabled())
		BlinkChargingState(&gauge);
	
	// Shift gauge to start at RC2 and push changes to the IO pins.
	gauge <<= 2;
	LATC = ~(gauge);
}

/**
 * Blinks the current charging state in the battery gauge. This gives the user
 * some feedback about what's happening.
 * 
 * @param gauge Pointer to the gauge LEDs variable (RC0 referenced).
 */
inline void BlinkChargingState(uint8_t *gauge) {
	// Determine which bit to flip, making sure we are only getting the first 4 bits
	switch (*gauge & 0b1111) {
		case 0b0000:
			*gauge ^= blinkState;
			break;
		case 0b0001:
			*gauge ^= (blinkState << 1);
			break;
		case 0b0011:
			*gauge ^= (blinkState << 2);
			break;
		case 0b0111:
			*gauge ^= (blinkState << 3);
			break;
	}

	// Flip the blink flag.
	blinkState ^= 1;
}