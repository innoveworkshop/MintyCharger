/* 
 * vreg.h
 * A voltage regulation module for the DC/DC boost circuit. This will simulate
 * a CV/CC supply.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef VREG_H
#define	VREG_H

#include <stdint.h>

// Battery status.
uint16_t GetBatteryVoltage(void);

// Voltage/current regulation.
void SetTargetVoltage(const float voltage);
void SetTargetCurrent(const float current);
void RegulateBoostOutput(void);

// ADC stuff.
void AcquireADC(const uint8_t channel);
void StoreADCValue(const uint16_t adcSample);
void StartNextADCReading(void);

// PWM stuff.
void SetPWMDutyCycle(const uint16_t pwmDutyCycle);

#endif	/* VREG_H */

