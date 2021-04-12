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
mode_t GetChargerModeSetting(void);
void SaveChargerModeSetting(const mode_t mode);

// Charge rate.
rate_t GetChargeRateSetting(void);
void SaveChargeRateSetting(const rate_t rate);

// Battery type.
battery_t GetBatteryTypeSetting(void);
void SaveBatteryTypeSetting(const battery_t type);

#endif	/* EEPROM_H */
