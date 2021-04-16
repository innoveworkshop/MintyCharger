/**
 * adc.h
 * Abstraction layer to handle everything related to measurements and ADCs.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef ADC_H
#define	ADC_H

#include <stdbool.h>
#include <stdint.h>

// Public definitions.
#define ADC_VREF_VOLTAGE 2.048f         // V
#define ADC_RESOLUTION   1023.0f
#define VSENSE_VDIV      0.16666666666f // R1 = 10k - R2 = 2k
#define ISENSE_GAIN      11

// Direct ADC interaction.
extern inline void __attribute__((always_inline)) AcquireADC(const uint8_t channel);
extern inline void __attribute__((always_inline)) StoreADCValue(const uint16_t adcSample);
extern inline void __attribute__((always_inline)) StartNextADCReading(void);

// Battery state.
float GetBatteryVoltage(void);
float GetBatteryCurrent(void);
float GetCellVoltage(void);
extern inline bool __attribute__((always_inline)) IsBatteryDisconnected(void);

// ADC values.
extern inline uint16_t __attribute__((always_inline)) GetBatteryVoltageValue(void);
extern inline uint16_t __attribute__((always_inline)) GetMeasuredVoltageValue(void);
extern inline uint16_t __attribute__((always_inline)) GetMeasuredCurrentValue(void);

#endif	/* ADC_H */

