/**
 * MintyCharger
 * A complete 9V (9.6V and 8.4V) NiMH charger that fits neatly inside an Altoids
 * tin.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "vreg.h"
#include "interface.h"

// Function prototypes.
void EnableInterrupts(void);
void DisableInterrupts(void);
void InitializeIO(void);
void InitializeADC(void);
void InitializePWM(void);
void InitializeButtonHoldTimer(void);

/**
 * Application main entry point.
 */
void main(void) {
	// Initialize everything.
	DisableInterrupts();
	InitializeIO();
	InitializeADC();
	InitializePWM();
	InitializeButtonHoldTimer();
	__delay_ms(100);              // Give some time for stuff to stabilize.
	EnableInterrupts();
	
	// Start the voltage regulation ADC loop.
	DisableRegulator();
	StartNextADCReading();
	InitializeUI();
	
	// Main application loop.
	while (true) {
	}
}

/**
 * Interrupt service routine.
 */
void __interrupt() ISR(void) {
	// ADC interrupt.
	if (PIR1bits.ADIF) {
		// Combine registers into a single 10-bit value and store it.
		uint16_t adcValue = (ADRESH << 8) | ADRESL;
		StoreADCValue(adcValue);
		
		// Regulate the boost output voltage and current.
		RegulateBoostOutput();
		
		// Clear the interrupt and start the next reading cycle.
		PIR1bits.ADIF = 0;
		StartNextADCReading();
	}
	
	// Select button (INT) interrupt.
	if (PIR0bits.INTF) {
		// Check if the button is still pressed.
		if ((PORTA & BTN_SELECT) == 0) {
			// Restart the hold down timer.
			T1CONbits.TMR1ON = 0; // Disable the timer.
			TMR1H = 0;            // Reset the high part of the counter.
			TMR1L = 0;            // Reset the low part of the counter.
			T1CONbits.TMR1ON = 1; // Enable the timer.
		} else {
			// Check if the timer is running. Detect if it was a single click.
			if (T1CONbits.TMR1ON) {
				// Select the next option of the current selection.
				SelectNextOption();
			}
			
			// Disable the timer.
			T1CONbits.TMR1ON = 0;
		}
		
		// Clear the interrupt.
		PIR0bits.INTF = 0;
	}
	
	// Button hold timer (Timer1) interrupt.
	if (PIR1bits.TMR1IF) {
		// Check if the button is still pressed.
		if ((PORTA & BTN_SELECT) == 0) {
			// Go to the next selection.
			NextConfigurationSelection();
		}
		
		// Turn the timer off and clear the interrupt.
		T1CONbits.TMR1ON = 0;
		PIR1bits.TMR1IF = 0;
	}
}

/**
 * Sets up all the I/O pins for the application.
 */
void InitializeIO(void) {
	// Setup digital inputs.
	TRISA = BTN_SELECT;
	TRISC = VSENSE + ISENSE;
	
	// Disable ALL analog inputs on PORTA.
	ANSELA = 0;
	
	// Setup pull-ups.
	WPUA = 0;
	WPUC = 0;
	
	// Setup open-drains.
	ODCONA = 0;
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
	RA5PPSbits.RA5PPS = 0b10;     // Set PWM5 output to RA5.
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
	PIE1bits.TMR2IE = 0;   // Disable its interrupt.
	T2CONbits.T2CKPS = 0;  // Prescaler set to 1.
	T2CONbits.TMR2ON = 1;  // Enable the timer.
	
	// Wait for Timer2 to be ready.
	while (TMR2IF == 0);
	
	// Enable the PWM.
	PWM5CONbits.PWM5EN = 1;
}

/**
 * Initializes the timer that will be used to determine a button hold.
 */
void InitializeButtonHoldTimer(void) {
	// Unlock the PPS and set the Select button as a source for the INT interrupt.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;    // Unlock the PPS.
	INTPPSbits.INTPPS = 0b00011;  // Set RA3 as an input to INT.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1;    // Lock the PPS.
	INTCONbits.INTEDG = 0;        // Trigger on the falling edge.
	PIE0bits.INTE = 1;            // Enable the INT interrupt.
	
	// Setup the Timer1 as a button hold timer of ~2s.
	T1CONbits.TMR1ON = 0;     // Disable the timer.
	T1CONbits.TMR1CS = 0b11;  // LFINTOSC as the clock source.
	T1CONbits.T1CKPS = 0b00;  // Prescaler set to 1.
	T1GCONbits.TMR1GE = 0;    // Disable gating. Will run like any other timer.
	PIE1bits.TMR1IE = 1;      // Enable its interrupt.
}

/**
 * Enables global interrupts.
 * @remark You should set the interrupts of individual peripherals yourself.
 *         This function will only enable them globally.
 */
void EnableInterrupts(void) {
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
}

/**
 * Disables global interrupts.
 * @remark You should set the interrupts of individual peripherals yourself.
 *         This function will only disable them globally.
 */
void DisableInterrupts(void) {
	INTCONbits.GIE = 0;
	INTCONbits.PEIE = 0;
}
