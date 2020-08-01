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

// Function prototypes.
void InitializeIO();
void InitializeADC();
void InitializePWM();
void SetPWMDutyCycle(uint16_t duty_cycle);

/**
 * Application main entry point.
 */
void main(void) {
	uint8_t pins = CHG_LED0;
	uint16_t pwm = 0;
	uint16_t res = 0;
	uint16_t vset = (10.0f * (2.0f / 12.0f)) / (2.048f / 1023.0f);
	
	// Initialize everything.
	InitializeIO();
	InitializeADC();
	InitializePWM();
	
	while (true) {
		// Read the ADC value.
		__delay_us(10);     // Acquisition delay.
		ADCON0bits.GO = 1;  // Start conversion.
		while (ADCON0bits.GO_nDONE);  // Wait for the conversion to end.
		res = (ADRESH << 8) | ADRESL;  // Store the result.
		
		// Control the PWM.
		if (res < vset) {
			if (pwm < 253) {
				pwm++;
			} else {
				pwm = 0;
			}
		} else if (res > vset) {
			if (pwm > 0) {
				pwm--;
			}
		}
		SetPWMDutyCycle(pwm);
		
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
 * Sets up all the I/O pins for the application.
 */
void InitializeIO() {
	// Setup digital inputs.
	TRISA = 0b111 + BTN_SELECT;
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
void InitializeADC() {
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
}

/**
 * Sets up the PWM module and assigns the output pin.
 */
void InitializePWM() {
	// Unlock the PPS and set the PWM output pin.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;    // Unlock the PPS.
	//RA5PPSbits.RA5PPS = 0b01100;  // Set CCP1 output to RA5.
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
 * Sets the PWM duty cycle.
 * 
 * @param duty_cycle 10-bit value that represents the duty cycle.
 */
void SetPWMDutyCycle(uint16_t duty_cycle) {
	PWM5DCL = (uint8_t)(duty_cycle << 6);
	PWM5DCH = (duty_cycle & 0b1111111100);
}