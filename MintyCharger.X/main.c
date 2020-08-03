/**
 * MintyCharger
 * A complete 9V (9.6V and 8.4V) NiMH charger that fits neatly inside an Altoids
 * tin.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

//                           16F18325
//                         +----------+
//                        -|RA0    RC0|- Voltage Sense - ANC0
//                        -|RA1    RC1|- Current Sense - ANC1
// Serial TX              -|RA2    RC2|- Charge LED 0
// Select Switch          -|RA3    RC3|- Charge LED 1
// Discharge Enable       -|RA4    RC4|- Charge LED 2
// Boost DC/DC PWM - PWM1 -|RA5    RC5|- Charge LED 3
//                         +----------+

#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "vreg.h"
#include "serial.h"

// Function prototypes.
void EnableInterrupts(const uint8_t enable);
void InitializeIO(void);
void InitializeADC(void);
void InitializePWM(void);

/**
 * Application main entry point.
 */
void main(void) {
	uint8_t pins = CHG_LED0;
	
	// Initialize everything.
	EnableInterrupts(0);
	InitializeIO();
	InitializeADC();
	InitializePWM();
	InitializeSerial();
	EnableInterrupts(1);
	
	// Start the voltage regulation loop.
	SetTargetVoltage(10.0f);
	SetTargetCurrent(0.05f);
	StartNextADCReading();
	
	// Main application loop.
	while (true) {
		if ((PORTA & BTN_SELECT) == 0) {
			LATC = 0;
		} else {
			LATC = ~pins;
		}
		
		if (pins < CHG_LED3) {
			pins <<= 1;
		} else {
			pins = CHG_LED0;
		}
	}

	return;
}

/**
 * Interrupt service routine.
 */
void __interrupt() ISR(void) {
	if (PIR1bits.ADIF == 1) {
		// ADC interrupt.
		uint16_t adcValue = (ADRESH << 8) | ADRESL;
		StoreADCValue(adcValue);
		
		// Regulate the boost output voltage and current.
		RegulateBoostOutput();
		
		// Clear the interrupt and start the next reading cycle.
		PIR1bits.ADIF = 0;
		StartNextADCReading();
	}
}

/**
 * Sets up all the I/O pins for the application.
 */
void InitializeIO(void) {
	// Setup digital inputs.
	TRISA = BTN_SELECT;
	TRISC = VSENSE + ISENSE;
	
	// Setup pull-ups.
	WPUC = 0;
	
	// Setup open-drains.
	ODCONC = CHG_LED0 + CHG_LED1 + CHG_LED2 + CHG_LED3;
	
	// Set all the outputs to LOW.
	LATA = 0;
	LATC = 0;
}

/**
 * Sets up the ADCs and their associated pins.
 */
void InitializeADC(void) {
	// Setup pins for the ADC.
	ANSELC = VSENSE + ISENSE;
	
	// Setup the FVR module.
	FVRCONbits.ADFVR = 0b10;  // 2.048V ADC reference.
	FVRCONbits.FVREN = 1;     // Enable the reference module.
	FVRCONbits.TSEN = 0;      // Disable the temperature indicator.
	
	// Setup the ADC module.
	ADCON1bits.ADFM = 1;             // Right justified.
	ADCON1bits.ADCS = 0b010;         // Fosc/32 (1us @ 32MHz)
	ADCON1bits.ADNREF = 0;           // Negative reference is Vss.
	ADCON1bits.ADPREF = 0b11;        // Positive reference is connected to the FVR.
	ADCON0bits.CHS = ADC_CH_VSENSE;  // Set the VSENSE pin as the default channel.
	ADCON0bits.ADON = 1;             // Turn the ADC module ON.
	PIE1bits.ADIE = 1;               // Enable the ADC interrupt.
}

/**
 * Sets up the PWM module and assigns the output pin.
 */
void InitializePWM(void) {
	// Unlock the PPS and set the PWM output pin.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;    // Unlock the PPS.
	RA5PPSbits.RA5PPS = 0b10;   // Set PWM5 output to RA5.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1;    // Lock the PPS.
	
	// Setup the PWM module.
	PWM5CONbits.PWM5POL = 0;   // Pulses are positive.
	PR2 = 0x3F;                // 78.12kHz @ 20MHz
	PWMTMRSbits.P5TSEL = 0b01; // Use Timer2.
	PWM5DCL = 0;               // Clear the duty cycle registers.
	PWM5DCH = 0;
	
	// Setup the Timer2 for PWM operation.
	PIR1bits.TMR2IF = 0;   // Disable its interrupt.
	T2CONbits.T2CKPS = 0;  // Prescaler set to 1.
	T2CONbits.TMR2ON = 1;  // Enable the timer.
	
	// Wait for Timer2 to be ready.
	while (TMR2IF == 0);
	
	// Enable the PWM.
	PWM5CONbits.PWM5EN = 1;
}

/**
 * Enable/disables global interrupts.
 * @remark You should set the interrupts of individual peripherals yourself.
 *         This function will only enable/disable them globally.
 * 
 * @param enable Should we enable them?
 */
void EnableInterrupts(const uint8_t enable) {
	INTCONbits.GIE = enable;
	INTCONbits.PEIE = enable;
}
