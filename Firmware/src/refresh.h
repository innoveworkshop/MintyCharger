/**
 * refresh.h
 * Controls the operation of the battery "refresher".
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef REFRESH_H
#define	REFRESH_H

// Public enumerators.
typedef enum {
	REFRESH_STOPPED, REFRESH_DISCHARGING, REFRESH_CHARGING
} refresh_t;

// Cycle control.
void InitializeRefreshCycle(void);
void StopRefreshCycle(void);
void DetectRefreshCycleSwitch(void);
void NextRefreshCyclePart(void);

// Cycle status.
extern inline refresh_t __attribute__((always_inline)) GetRefreshCycle(void);

#endif	/* REFRESH_H */
