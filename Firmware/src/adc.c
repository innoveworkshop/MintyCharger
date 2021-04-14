/**
 * adc.c
 * Abstraction layer to handle everything related to measurements and ADCs.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "adc.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "vreg.h"
#include "interface.h"

// Private definitions.
#define ADC_ACQ_DELAY       5   // us
#define BATT_IDISCONNECT    10  // ~2mA
#define USB_LEAKAGE_VOLTAGE 466 // ~5.6V

// Private variables.
uint8_t adcLastChannel = 0;
uint16_t adcVoltage    = 0;
uint16_t adcCurrent    = 0;

/**
 * Starts an ADC acquisition of a specified channel.
 * @remark The acquired value will later be stored when the ADC interrupt
 *         happens and the StoreADCValue function is called.
 * 
 * @param channel ADC channel to be sampled.
 */
inline void AcquireADC(const uint8_t channel) {
	// Switch to Vss first to make sure we can accurately read a smaller voltage.
	ADCON0bits.CHS = 0b111100;
	__delay_us(1); // Let the hold capacitor discharge.

	// Set the ADC channel and store it for later.
	ADCON0bits.CHS = channel;
	adcLastChannel = channel;

	// Start an acquisition.
	__delay_us(ADC_ACQ_DELAY); // Acquisition delay.
	ADCON0bits.GO = 1; // Start conversion.
}

/**
 * Stores the ADC sample of the last acquisition.
 * 
 * @param adcSample The ADC sample to be stored.
 */
inline void StoreADCValue(const uint16_t adcSample) {
	// Store the sample according to the last sampled channel.
	switch (adcLastChannel) {
		case ADC_CH_VSENSE:
			adcVoltage = adcSample;
			break;
		case ADC_CH_ISENSE:
			adcCurrent = adcSample;
			break;
	}
}

/**
 * Starts the acquisition cycle for the next ADC channel.
 */
inline void StartNextADCReading(void) {
	switch (adcLastChannel) {
		case ADC_CH_VSENSE:
			AcquireADC(ADC_CH_ISENSE);
			return;
		case 0:
		case ADC_CH_ISENSE:
			AcquireADC(ADC_CH_VSENSE);
			return;
	}
}

/**
 * Gets the battery voltage at the output of the voltage regulator.
 * 
 * @return Current battery voltage.
 */
float GetBatteryVoltage(void) {
	return (GetBatteryVoltageValue() * (ADC_VREF_VOLTAGE / ADC_RESOLUTION)) / VSENSE_VDIV;
}

/**
 * Gets the battery current.
 * 
 * @return Battery current.
 */
float GetBatteryCurrent(void) {
	return (GetMeasuredCurrentValue() * (ADC_VREF_VOLTAGE / ADC_RESOLUTION)) / ISENSE_GAIN;
}

/**
 * Gets the estimate of the individual cell voltages inside the battery.
 * 
 * @return Individual cell voltage.
 */
float GetCellVoltage(void) {
	uint8_t numCells = 1;

	switch (GetSelectedBattery()) {
		case NIMH_72V:
			numCells = 6;
			break;
		case LTION_74V:
			numCells = 2;
			break;
		case NIMH_84V:
			numCells = 7;
			break;
		case NIMH_96V:
			numCells = 8;
			break;
	}

	return GetBatteryVoltage() / (float) numCells;
}

/**
 * Check if the battery has been disconnected.
 * 
 * @return TRUE if the battery has been disconnected.
 */
inline bool IsBatteryDisconnected(void) {
	// Check if there's more than USB voltage present in case of a lithium battery.
	if (IsLithiumBattery() && (GetMeasuredVoltageValue() > USB_LEAKAGE_VOLTAGE))
		return false;

	return GetMeasuredCurrentValue() < BATT_IDISCONNECT;
}

/**
 * Gets the ADC value of the voltage across the battery.
 * 
 * @return ADC value of the battery voltage.
 */
inline uint16_t GetBatteryVoltageValue(void) {
	return adcVoltage - (adcCurrent / ISENSE_GAIN);
}

/**
 * Gets the measured voltage ADC value.
 * 
 * @return Measured voltage ADC value.
 */
inline uint16_t GetMeasuredVoltageValue(void) {
	return adcVoltage;
}

/**
 * Gets the measured current ADC value.
 * 
 * @return Measured current ADC value.
 */
inline uint16_t GetMeasuredCurrentValue(void) {
	return adcCurrent;
}