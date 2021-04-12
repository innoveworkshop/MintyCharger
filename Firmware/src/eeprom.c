/**
 * eeprom.c
 * Abstracts away all of the EEPROM related tasks.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "eeprom.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>

// EEPROM locations.
#define EEPROM_CHARGE_MODE 0x00
#define EEPROM_CHARGE_RATE 0x01
#define EEPROM_BATT_TYPE   0x02

/**
 * EEPROM default values.
 *
 * 0x00 -> Charger mode   = MODE_CHARGE
 * 0x01 -> Charge current = RATE_50MA
 * 0x02 -> Charge voltage = NIMH_84V
 */
__EEPROM_DATA(0x00, 0x01, 0x02, 0, 0, 0, 0, 0);

/**
 * Retrieves the charger mode configuration from the EEPROM.
 * 
 * @return Charger mode configuration.
 */
inline mode_t GetChargerModeSetting(void) {
	return (mode_t) eeprom_read(EEPROM_CHARGE_MODE);
}

/**
 * Sets the charger mode configuration in the EEPROM.
 * 
 * @param mode Charger mode.
 */
inline void SaveChargerModeSetting(const mode_t mode) {
	eeprom_write(EEPROM_CHARGE_MODE, (uint8_t) mode);
}

/**
 * Retrieves the charge rate configuration from the EEPROM.
 * 
 * @return Charge rate configuration.
 */
inline rate_t GetChargeRateSetting(void) {
	return (rate_t) eeprom_read(EEPROM_CHARGE_RATE);
}

/**
 * Sets the charge rate configuration in the EEPROM.
 * 
 * @param rate Charge rate.
 */
inline void SaveChargeRateSetting(const rate_t rate) {
	eeprom_write(EEPROM_CHARGE_RATE, (uint8_t) rate);
}

/**
 * Retrieves the battery type configuration from the EEPROM.
 * 
 * @return Battery type configuration.
 */
inline battery_t GetBatteryTypeSetting(void) {
	return (battery_t) eeprom_read(EEPROM_BATT_TYPE);
}

/**
 * Sets the battery type configuration in the EEPROM.
 * 
 * @param type Battery type.
 */
inline void SaveBatteryTypeSetting(const battery_t type) {
	eeprom_write(EEPROM_BATT_TYPE, (uint8_t) type);
}