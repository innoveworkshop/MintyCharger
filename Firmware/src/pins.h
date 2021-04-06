/* 
 * pins.h
 * Pin definitions.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef PINS_H
#define	PINS_H

//                           16F18325
//                         +----------+
// LED Shift Register DAT -|RA0    RC0|- Voltage Sense - ANC0
// LED Shift Register CLK -|RA1    RC1|- Current Sense - ANC1
// LED Shift Register LAT -|RA2    RC2|- Charge LED 0
// Select Switch          -|RA3    RC3|- Charge LED 1
// Discharge Enable       -|RA4    RC4|- Charge LED 2
// Boost DC/DC PWM - PWM1 -|RA5    RC5|- Charge LED 3
//                         +----------+


// Pin definitions.
#define DISCH_VSET 1         // RA0
#define SR_DATA    (1 << 1)  // RA1
#define SR_CLOCK   (1 << 2)  // RA2
#define BTN_SELECT (1 << 3)  // RA3
#define SR_LATCH   (1 << 4)  // RA4
#define BOOST_PWM  (1 << 5)  // RA5
#define VSENSE     1         // RC0
#define ISENSE     (1 << 1)  // RC1
#define CHG_LED0   (1 << 2)  // RC2
#define CHG_LED1   (1 << 3)  // RC3
#define CHG_LED2   (1 << 4)  // RC4
#define CHG_LED3   (1 << 5)  // RC5

// Analog channel definitions.
#define ADC_CH_VSENSE 0b10000
#define ADC_CH_ISENSE 0b10001

#endif	/* PINS_H */
