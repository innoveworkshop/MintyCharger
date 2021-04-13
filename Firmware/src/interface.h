/**
 * interface.h
 * Handles all the user interface stuff.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef INTERFACE_H
#define	INTERFACE_H

#include <stdbool.h>

// Public enumarators.
typedef enum {
	NIMH_72V, LTION_74V, NIMH_84V, NIMH_96V
} battery_t;

typedef enum {
	RATE_15MA, RATE_50MA, RATE_75MA, RATE_100MA, RATE_TRICKLE
} rate_t;

typedef enum {
	MODE_CHARGE, MODE_DISCHARGE, MODE_REFRESH
} mode_t;

// Selecting configurations.
void SelectNextOption(void);
void NextConfigurationSelection(void);
void FlashCurrentEditableConfiguration(void);

// User interaction.
void HandleSingleButtonClick(void);

// Initialization.
void InitializeUI(void);

// Getting configurations.
extern inline mode_t __attribute__((always_inline)) GetSelectedMode(void);
extern inline battery_t __attribute__((always_inline)) GetSelectedBattery(void);
extern inline rate_t __attribute__((always_inline)) GetSelectedCurrent(void);
extern inline bool __attribute__((always_inline)) IsLithiumBattery(void);
extern inline bool __attribute__((always_inline)) IsSelectingConfiguration(void);

#endif	/* INTERFACE_H */
