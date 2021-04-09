/* 
 * vreg.c
 * A voltage regulation module for the DC/DC boost circuit. This will simulate
 * a CV/CC supply.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "vreg.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "interface.h"

// Some definitions.
#define PWM_MAX_VALUE      1022
#define PWM_INC_STEP       1
#define PWM_DEC_STEP       1
#define ADC_ACQ_DELAY      5       // us
#define VREF_VOLTAGE       2.048f  // V
#define ADC_RESOLUTION     1023.0f
#define VSENSE_VDIV        (2.0f / 12.0f)
#define ISENSE_GAIN        11
#define NIMH_ICUTOFF       25  // ~5mA
#define LTION_ICUTOFF      55  // ~10mA
#define BATT_IDISCONNECT   10  // ~2mA

// Private variables.
bool enabled            = false;
uint16_t pwmValue       = 0;
uint16_t adcVoltage     = 0;
uint16_t adcCurrent     = 0;
uint8_t adcLastChannel  = 0;
uint16_t targetVoltage  = 0;
uint16_t targetCurrent  = 0;
bool finishedCharging   = true;

/**
 * Enables the voltage regulator.
 */
void EnableRegulator(void) {
	pwmValue = 0;
	enabled = true;
}

/**
 * Disables the voltage regulator.
 */
void DisableRegulator(void) {
	pwmValue = 0;
	SetPWMDutyCycle(pwmValue);
	enabled = false;
}

/**
 * Regulates the voltage output of the boost converter to make sure it respects
 * its limits.
 */
void RegulateBoostOutput(void) {
	if (!enabled)
		return;

	// Control the PWM in order to maintain regulation.
	if ((GetBatteryVoltageValue() < targetVoltage) &&
			(adcCurrent < targetCurrent)) {
		// Ramp up the voltage.
		if (pwmValue < (PWM_MAX_VALUE - PWM_INC_STEP)) {
			pwmValue += PWM_INC_STEP;
		} else {
			pwmValue = PWM_MAX_VALUE;
		}
	} else if ((GetBatteryVoltageValue() > targetVoltage) ||
			(adcCurrent > targetCurrent)) {
		// Ramp down the voltage.
		if (pwmValue > PWM_DEC_STEP) {
			pwmValue -= PWM_DEC_STEP;
		} else {
			pwmValue = 0;
		}
	}
	
	// Set the PWM duty cycle.
	SetPWMDutyCycle(pwmValue);
}

/**
 * Starts an ADC acquisition of a specified channel.
 * @remark The acquired value will later be stored when the ADC interrupt
 *         happens and the StoreADCValue function is called.
 * 
 * @param channel ADC channel to be sampled.
 */
void AcquireADC(const uint8_t channel) {
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
void StoreADCValue(const uint16_t adcSample) {
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
void StartNextADCReading(void) {
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
 * Detects the end of charge of a battery and acts upon it.
 */
void DetectEndOfCharge(void) {
	// Detect the end of charge.
	if (IsLithiumBattery()) {
		if (GetMeasuredCurrentValue() < LTION_ICUTOFF)
			SetFinishedCharging();
	} else {
		if (GetMeasuredCurrentValue() < NIMH_ICUTOFF)
			SetFinishedCharging();
	}
}

/**
 * Check if the battery has been disconnected.
 * 
 * @return TRUE if the battery has been disconnected.
 */
bool IsBatteryDisconnected(void) {
	// Check if there's >5.6V voltage present in case of a lithium battery.
	if (IsLithiumBattery() && (GetMeasuredVoltageValue() > 466))
		return false;
	
	return GetMeasuredCurrentValue() < BATT_IDISCONNECT;
}

/**
 * Sets the PWM duty cycle.
 * 
 * @param pwmDutyCycle 10-bit value that represents the duty cycle.
 */
void SetPWMDutyCycle(const uint16_t pwmDutyCycle) {
	PWM5DCL = (uint8_t) (pwmDutyCycle << 6);
	PWM5DCH = pwmDutyCycle >> 2;
}

/**
 * Sets the regulator target voltage.
 * 
 * @param voltage Target voltage.
 */
void SetTargetVoltage(const float voltage) {
	targetVoltage = (voltage * VSENSE_VDIV) / (VREF_VOLTAGE / ADC_RESOLUTION);
}

/**
 * Sets the regulator target current.
 * 
 * @param current Target current.
 */
void SetTargetCurrent(const float current) {
	targetCurrent = (current * ISENSE_GAIN) / (VREF_VOLTAGE / ADC_RESOLUTION);
}

/**
 * Finishes the charging process.
 */
void SetFinishedCharging(void) {
	finishedCharging = true;

	// Completely stop charging if it is a lithium or disconnected battery.
	if (IsLithiumBattery() || IsBatteryDisconnected())
		DisableRegulator();
}

/**
 * Clears the finished charging flag.
 */
void ClearFinishedCharging(void) {
	finishedCharging = false;
	EnableRegulator();
}

/**
 * Gets the target voltage ADC value.
 * 
 * @return Target voltage ADC value.
 */
uint16_t GetTargetVoltageValue(void) {
	return targetVoltage;
}

/**
 * Gets the target current ADC value.
 * 
 * @return Target current ADC value.
 */
uint16_t GetTargetCurrentValue(void) {
	return targetCurrent;
}

/**
 * Gets the measured voltage ADC value.
 * 
 * @return Measured voltage ADC value.
 */
uint16_t GetMeasuredVoltageValue(void) {
	return adcVoltage;
}

/**
 * Gets the measured current ADC value.
 * 
 * @return Measured current ADC value.
 */
uint16_t GetMeasuredCurrentValue(void) {
	return adcCurrent;
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
 * Gets the battery voltage at the output of the voltage regulator.
 * 
 * @return Current battery voltage.
 */
float GetBatteryVoltage(void) {
	return (GetBatteryVoltageValue() * (VREF_VOLTAGE / ADC_RESOLUTION)) / VSENSE_VDIV;
}

/**
 * Gets the battery current.
 * 
 * @return Battery current.
 */
float GetBatteryCurrent(void) {
	return (GetMeasuredCurrentValue() * (VREF_VOLTAGE / ADC_RESOLUTION)) / ISENSE_GAIN;
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
 * Checks if the regulator is currently in constant current mode.
 * 
 * @return Is in constant current mode?
 */
bool IsConstantCurrent(void) {
	return GetMeasuredCurrentValue() < GetTargetCurrentValue();
}

/**
 * Checks if the charge cycle has finished.
 * 
 * @return Have we finished charging?
 */
bool IsFinishedCharging(void) {
	return finishedCharging;
}