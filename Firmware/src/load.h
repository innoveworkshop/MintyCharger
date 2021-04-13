/**
 * load.h
 * Electronic load to discharge the battery.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef LOAD_H
#define	LOAD_H

#include <stdint.h>

// Electronic load control.
void EnableLoad(void);
void DisableLoad(void);

// Load current.
float GetLoadCurrent(void);
void SetLoadCurrent(const float current);

#endif	/* LOAD_H */