/**
 * load.h
 * Electronic load to discharge the battery.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef LOAD_H
#define	LOAD_H

#include <stdbool.h>
#include <stdint.h>

// Electronic load control.
void EnableLoad(void);
void DisableLoad(void);
void DetectLoadCutoff(void);
extern inline bool __attribute__((always_inline)) IsLoadEnabled(void);

// Cutoff detection.
bool IsAtCutoffVoltage(void);

// Load current.
float GetLoadCurrent(void);
void SetLoadCurrent(const float current);

#endif	/* LOAD_H */