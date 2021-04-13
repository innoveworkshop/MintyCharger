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
#include "adc.h"
#include "interface.h"
#include "load.h"

// Some definitions.
#define PWM_MAX_VALUE       1022
#define PWM_INC_STEP        1
#define PWM_DEC_STEP        1
#define NIMH_ICUTOFF        25  // ~5mA
#define LTION_ICUTOFF       55  // ~10mA
#define MEAN_CURRENT_CYCLES 5

// Private variables.
bool enabled           = false;
uint16_t pwmValue      = 0;
uint16_t targetVoltage = 0;
uint16_t targetCurrent = 0;
bool finishedCharging  = true;
uint32_t meanCurrent   = 0;
uint8_t meanICounter   = 0;

/**
 * Enables the voltage regulator.
 */
void EnableRegulator(void) {
	// Disable the electronic load.
	DisableLoad();
	
	// Reset PWM duty cycle and set the enabled flag.
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
	if ((GetBatteryVoltageValue() < GetTargetVoltageValue()) &&
			(GetMeasuredCurrentValue() < GetTargetCurrentValue())) {
		// Ramp up the voltage.
		if (pwmValue < (PWM_MAX_VALUE - PWM_INC_STEP)) {
			pwmValue += PWM_INC_STEP;
		} else {
			pwmValue = PWM_MAX_VALUE;
		}
	} else if ((GetBatteryVoltageValue() > GetTargetVoltageValue()) ||
			(GetMeasuredCurrentValue() > GetTargetCurrentValue())) {
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
 * Detects the end of charge of a battery and acts upon it.
 */
void DetectEndOfCharge(void) {
	// Get the mean of the current over a period of time.
	if (meanICounter < MEAN_CURRENT_CYCLES) {
		meanCurrent += GetMeasuredCurrentValue();
		meanICounter++;
		
		return;
	}
	
	// Get the mean of the gathered currents.
	meanCurrent /= MEAN_CURRENT_CYCLES;
		
	// Detect the end of charge.
	if (IsLithiumBattery()) {
		if (meanCurrent < LTION_ICUTOFF)
			SetFinishedCharging();
	} else {
		if (meanCurrent < NIMH_ICUTOFF)
			SetFinishedCharging();
	}
	
	// Reset everything.
	meanCurrent = 0;
	meanICounter = 0;
}

/**
 * Sets the PWM duty cycle.
 * 
 * @param pwmDutyCycle 10-bit value that represents the duty cycle.
 */
inline void SetPWMDutyCycle(const uint16_t pwmDutyCycle) {
	PWM5DCL = (uint8_t)(pwmDutyCycle << 6);
	PWM5DCH = (uint8_t)(pwmDutyCycle >> 2);
}

/**
 * Sets the regulator target voltage.
 * 
 * @param voltage Target voltage.
 */
void SetTargetVoltage(const float voltage) {
	targetVoltage = (uint16_t)((voltage * VSENSE_VDIV) / (ADC_VREF_VOLTAGE / ADC_RESOLUTION));
}

/**
 * Sets the regulator target current.
 * 
 * @param current Target current.
 */
void SetTargetCurrent(const float current) {
	targetCurrent = (uint16_t)((current * ISENSE_GAIN) / (ADC_VREF_VOLTAGE / ADC_RESOLUTION));
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
inline uint16_t GetTargetVoltageValue(void) {
	return targetVoltage;
}

/**
 * Gets the target current ADC value.
 * 
 * @return Target current ADC value.
 */
inline uint16_t GetTargetCurrentValue(void) {
	return targetCurrent;
}

/**
 * Checks if the regulator is currently in constant current mode.
 * 
 * @return Is in constant current mode?
 */
inline bool IsConstantCurrent(void) {
	return GetMeasuredCurrentValue() < GetTargetCurrentValue();
}

/**
 * Checks if the charge cycle has finished.
 * 
 * @return Have we finished charging?
 */
inline bool IsFinishedCharging(void) {
	return finishedCharging;
}