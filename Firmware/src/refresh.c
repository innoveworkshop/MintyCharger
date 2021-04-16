/**
 * refresh.c
 * Controls the operation of the battery "refresher".
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "refresh.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "vreg.h"
#include "load.h"
#include "interface.h"

// Private variables.
refresh_t refreshCycle = REFRESH_STOPPED;

/**
 * Stops the battery refresh cycle.
 */
void StopRefreshCycle(void) {
	// Disable everything just to be sure.
	DisableLoad();
	ClearFinishedCharging();
	DisableRegulator();
	
	// Stop the cycle.
	refreshCycle = REFRESH_STOPPED;
}

/**
 * Go to the next part of the refresh cycle.
 */
void NextRefreshCyclePart(void) {
	// Check if the next part of the cycle is actually stopping everything.
	if (refreshCycle == REFRESH_CHARGING) {
		StopRefreshCycle();
		return;
	}
	
	// Move to the next cycle.
	refreshCycle++;
	
	switch (refreshCycle) {
		case REFRESH_DISCHARGING:
			DisableRegulator();
			EnableLoad();
			break;
		case REFRESH_CHARGING:
			DisableLoad();
			ClearFinishedCharging();
			break;
		case REFRESH_STOPPED:
			// Do nothing.
			break;
	}
}

/**
 * Detect when to switch to the next part of the refresh cycle.
 */
void DetectRefreshCycleSwitch(void) {
	switch (refreshCycle) {
		case REFRESH_STOPPED:
			// Just ignore this if we are not running a refresh cycle.
			return;
		case REFRESH_DISCHARGING:
			// Check if the load has finished doing its job.
			if (!IsLoadEnabled())
				NextRefreshCyclePart();
			break;
		case REFRESH_CHARGING:
			// Check if the charger has finished.
			if (IsFinishedCharging())
				NextRefreshCyclePart();
			break;
	}
}

/**
 * Gets the current refresh cycle state.
 * 
 * @return Refresh cycle state.
 */
inline refresh_t GetRefreshCycle(void) {
	return refreshCycle;
}