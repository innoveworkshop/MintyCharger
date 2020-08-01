/* 
 * pins.h
 * Pin definitions.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef PINS_H
#define	PINS_H

// Pin definitions.
#define BTN_SELECT (1 << 3)  // RA3
#define DISCH_EN   (1 << 4)  // RA4
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

