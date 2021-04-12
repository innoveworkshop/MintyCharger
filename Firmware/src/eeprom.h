/**
 * eeprom.h
 * Abstracts away all of the EEPROM related tasks.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef EEPROM_H
#define	EEPROM_H

#include "interface.h"

// Charger mode.
extern inline mode_t __attribute__((always_inline)) GetChargerModeSetting(void);
extern inline void __attribute__((always_inline)) SaveChargerModeSetting(const mode_t mode);

// Charge rate.
extern inline rate_t __attribute__((always_inline)) GetChargeRateSetting(void);
extern inline void __attribute__((always_inline)) SaveChargeRateSetting(const rate_t rate);

// Battery type.
extern inline battery_t __attribute__((always_inline)) GetBatteryTypeSetting(void);
extern inline void __attribute__((always_inline)) SaveBatteryTypeSetting(const battery_t type);

#endif	/* EEPROM_H */
