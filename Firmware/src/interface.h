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
typedef enum { NIMH_72V, LTION_74V, NIMH_84V, NIMH_96V } battery_t;

// Selecting configurations.
void SelectNextOption(void);
void NextConfigurationSelection(void);
void FlashCurrentEditableConfiguration(void);

// User interaction.
void HandleSingleButtonClick(void);

// Initialization.
void InitializeUI(void);

// Getting configurations.
battery_t GetSelectedBattery(void);
bool IsLithiumBattery(void);

#endif	/* INTERFACE_H */
