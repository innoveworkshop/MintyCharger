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

// Some definitions.
#define PWM_MAX_VALUE      1022
#define PWM_RAMP_UP_CYCLES 100
#define ADC_ACQ_DELAY      10  // us
#define VREF_VOLTAGE       2.048f
#define ADC_RESOLUTION     1023.0f
#define VSENSE_VDIV        2.0f / 12.0f
#define ISENSE_GAIN        11

// Private variables.
uint16_t pwmValue       = 0;
uint16_t adcVoltage     = 0;
uint16_t adcCurrent     = 0;
uint8_t  adcLastChannel = 0;
uint16_t targetVoltage  = 0;
uint16_t targetCurrent  = 0;
uint16_t pwmCycleDelay  = 0;

/**
 * Regulates the voltage output of the boost converter to make sure it respects
 * its limits.
 */
void RegulateBoostOutput(void) {
	// Control the PWM in order to maintain regulation.
	if ((GetBatteryVoltageValue() < targetVoltage) &&
			(adcCurrent < targetCurrent)) {
		if (pwmValue < PWM_MAX_VALUE) {
			// Delay the ramp up of the voltage a bit to get a more stable output.
			pwmCycleDelay++;
			if (pwmCycleDelay == PWM_RAMP_UP_CYCLES) {
				pwmValue++;
				pwmCycleDelay = 0;
			}
		} else {
			pwmValue = 0;
		}
	} else if ((GetBatteryVoltageValue() > targetVoltage) ||
			(adcCurrent > targetCurrent)) {
		if (pwmValue > 0)
			pwmValue--;
	}
	
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
	__delay_us(1);              // Let the hold capacitor discharge.
	
	// Set the ADC channel and store it for later.
	ADCON0bits.CHS = channel;
	adcLastChannel = channel;

	// Start an acquisition.
	__delay_us(ADC_ACQ_DELAY);  // Acquisition delay.
	ADCON0bits.GO = 1;          // Start conversion.
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
 * Sets the PWM duty cycle.
 * 
 * @param pwmDutyCycle 10-bit value that represents the duty cycle.
 */
void SetPWMDutyCycle(const uint16_t pwmDutyCycle) {
	PWM5DCL = (uint8_t)(pwmDutyCycle << 6);
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
	return (adcVoltage * (VREF_VOLTAGE / ADC_RESOLUTION)) / VSENSE_VDIV;
}

/**
 * Gets the battery current.
 * 
 * @return Battery current.
 */
float GetBatteryCurrent(void) {
	return (adcCurrent * (VREF_VOLTAGE / ADC_RESOLUTION)) / ISENSE_GAIN;
}
