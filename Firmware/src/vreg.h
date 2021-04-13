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

// Voltage/current regulation.
void EnableRegulator(void);
void DisableRegulator(void);
void RegulateBoostOutput(void);

// Charging process.
void SetFinishedCharging(void);
void ClearFinishedCharging(void);
void DetectEndOfCharge(void);
extern inline bool __attribute__((always_inline)) IsFinishedCharging(void);
extern inline bool __attribute__((always_inline)) IsConstantCurrent(void);

// PWM stuff.
extern inline void __attribute__((always_inline)) SetPWMDutyCycle(const uint16_t pwmDutyCycle);

// Target voltage and current.
void SetTargetVoltage(const float voltage);
void SetTargetCurrent(const float current);
extern inline uint16_t __attribute__((always_inline)) GetTargetVoltageValue(void);
extern inline uint16_t __attribute__((always_inline)) GetTargetCurrentValue(void);

#endif	/* VREG_H */
