/* 
 * vreg.h
 * A voltage regulation module for the DC/DC boost circuit. This will simulate
 * a CV/CC supply.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef VREG_H
#define	VREG_H

#include <stdbool.h>
#include <stdint.h>

// Battery status.
float GetBatteryVoltage(void);
float GetBatteryCurrent(void);
float GetCellVoltage(void);

// Voltage/current regulation.
void EnableRegulator(void);
void DisableRegulator(void);
void SetTargetVoltage(const float voltage);
void SetTargetCurrent(const float current);
void RegulateBoostOutput(void);
extern inline bool __attribute__((always_inline)) IsConstantCurrent(void);

// Charging process.
void SetFinishedCharging(void);
void ClearFinishedCharging(void);
extern inline bool __attribute__((always_inline)) IsBatteryDisconnected(void);
extern inline bool __attribute__((always_inline)) IsFinishedCharging(void);
void DetectEndOfCharge(void);

// ADC stuff.
extern inline void __attribute__((always_inline)) AcquireADC(const uint8_t channel);
extern inline void __attribute__((always_inline)) StoreADCValue(const uint16_t adcSample);
extern inline void __attribute__((always_inline)) StartNextADCReading(void);

// PWM stuff.
extern inline void __attribute__((always_inline)) SetPWMDutyCycle(const uint16_t pwmDutyCycle);

// ADC values.
extern inline uint16_t __attribute__((always_inline)) GetBatteryVoltageValue(void);
extern inline uint16_t __attribute__((always_inline)) GetTargetVoltageValue(void);
extern inline uint16_t __attribute__((always_inline)) GetTargetCurrentValue(void);
extern inline uint16_t __attribute__((always_inline)) GetMeasuredVoltageValue(void);
extern inline uint16_t __attribute__((always_inline)) GetMeasuredCurrentValue(void);

#endif	/* VREG_H */
