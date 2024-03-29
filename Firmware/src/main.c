/**
 * MintyCharger
 * A complete PP3 (9.6V, 8.4V, 7.4V, and 7.2V) NiMH charger that fits neatly
 * inside an Altoids tin.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "eeprom.h"
#include "pins.h"
#include "adc.h"
#include "vreg.h"
#include "interface.h"
#include "gauge.h"
#include "load.h"
#include "refresh.h"
#include "version.h"

// Private methods.
void EnableInterrupts(void);
void DisableInterrupts(void);
void InitializeIO(void);
void InitializeFVR(void);
void InitializeADC(void);
void InitializePWM(void);
void InitializeDAC(void);
void InitializeButtonInterrupt(void);
void InitializeButtonHoldTimer(void);
void InitializeFlashingTimer(void);

/**
 * Main entry point.
 */
void main(void) {
	// Set the WDT to 4s for initializing everything.
	WDTCONbits.WDTPS = 0b01100;
	CLRWDT();
	
	// Initialize everything.
	DisableInterrupts();
	InitializeIO();
	InitializeFVR();
	InitializeADC();
	InitializePWM();
	InitializeDAC();
	InitializeButtonInterrupt();
	InitializeButtonHoldTimer();
	InitializeFlashingTimer();
	__delay_ms(100); // Give some time for stuff to stabilise.
	DisplayFirmwareInformation();
	EnableInterrupts();

	// Start the voltage regulation ADC loop.
	DisableRegulator();
	DisableLoad();
	StartNextADCReading();
	InitializeUI();

	// Reset the WDT to 256ms for normal operation and enable the flashing timer.
	WDTCONbits.WDTPS = 0b01000;
	T6CONbits.TMR6ON = 1;

	// Main application loop.
	while (true) {
		// Reset the watchdog timer.
		CLRWDT();
		
		// Add a bit of delay to make sure things are stable.
		__delay_ms(10);
		
		// Detect the battery end of charge.
		if (!IsFinishedCharging())
			DetectEndOfCharge();
		
		// Detect the load cutoff voltage and stop over discharging the battery.
		DetectLoadCutoff();
		
		// Detect if we need to go to the next part of a refresh cycle.
		DetectRefreshCycleSwitch();
	}
}

/**
 * Interrupt service routine.
 */
void __interrupt() ISR(void) {
	// ADC interrupt.
	if (PIR1bits.ADIF) {
		// Combine registers into a single 10-bit value and store it.
		uint16_t adcValue = (uint16_t)((ADRESH << 8) | ADRESL);
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
			if (T1CONbits.TMR1ON)
				HandleSingleButtonClick();

			// Disable the timer.
			T1CONbits.TMR1ON = 0;
		}

		// Clear the interrupt.
		PIR0bits.INTF = 0;
	}

	// Button hold timer (Timer1) interrupt.
	if (PIR1bits.TMR1IF) {
		// Check if the button is still pressed and go to the next selection.
		if ((PORTA & BTN_SELECT) == 0)
			NextConfigurationSelection();

		// Turn the timer off and clear the interrupt.
		T1CONbits.TMR1ON = 0;
		PIR1bits.TMR1IF = 0;
	}

	// Editing configuration flash timer (Timer6) interrupt.
	if (PIR2bits.TMR6IF) {
		// Blinkenlights.
		FlashCurrentEditableConfiguration();
		DisplayBatteryGauge();

		// Clear the interrupt.
		PIR2bits.TMR6IF = 0;
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
 * Sets up the fixed voltage reference.
 */
void InitializeFVR(void) {
	// Setup FVR outputs.
	FVRCONbits.ADFVR = 0b10;  // 2.048V ADC reference.
	FVRCONbits.CDAFVR = 0b01; // 1.024V DAC and Comparator reference.
	
	// Make it work.
	FVRCONbits.FVREN = 1;     // Enable the reference module.
	FVRCONbits.TSEN = 0;      // Disable the temperature indicator.
}

/**
 * Sets up the ADCs and their associated pins.
 */
void InitializeADC(void) {
	// Setup pins for the ADC.
	ANSELC = VSENSE + ISENSE;

	// Setup the ADC module.
	ADCON1bits.ADFM = 1;            // Right justified.
	ADCON1bits.ADCS = 0b010;        // Fosc/32 (1us @ 32MHz)
	ADCON1bits.ADNREF = 0;          // Negative reference is Vss.
	ADCON1bits.ADPREF = 0b11;       // Positive reference is connected to the FVR.
	ADCON0bits.CHS = ADC_CH_VSENSE; // Set the VSENSE pin as the default channel.
	ADCON0bits.ADON = 1;            // Turn the ADC module ON.
	PIE1bits.ADIE = 1;              // Enable the ADC interrupt.
}

/**
 * Sets up the PWM module and assigns the output pin.
 */
void InitializePWM(void) {
	// Unlock the PPS and set the PWM output pin.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0; // Unlock the PPS.
	RA5PPSbits.RA5PPS = 0b10;  // Set PWM5 output to RA5.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1; // Lock the PPS.

	// Setup the PWM module.
	PWM5CONbits.PWM5POL = 0;   // Pulses are positive.
	PR2 = 0x3F;                // 78.12kHz @ 20MHz
	PWMTMRSbits.P5TSEL = 0b01; // Use Timer2.
	PWM5DCL = 0;               // Clear the duty cycle registers.
	PWM5DCH = 0;

	// Setup the Timer2 for PWM operation.
	PIE1bits.TMR2IE = 0;       // Disable its interrupt.
	T2CONbits.T2CKPS = 0;      // Prescaler set to 1.
	T2CONbits.TMR2ON = 1;      // Enable the timer.

	// Wait for Timer2 to be ready.
	while (TMR2IF == 0);

	// Enable the PWM and unlock the slew rate for the pin.
	PWM5CONbits.PWM5EN = 1;
	SLRCONAbits.SLRA5 = 0;
}

/**
 * Sets up the DAC for the electronic DC load.
 */
void InitializeDAC(void) {
	// Setup voltage sources.
	DACCON0bits.DAC1PSS = 0b10; // Positive reference is connected to the FVR.
	DACCON0bits.DAC1NSS = 0;    // Negative reference is Vss.
	
	// Enable the DAC and reset its output.
	DACCON0bits.DAC1EN = 1;
	DACCON1bits.DAC1R = 0;
	
	// Enable the DAC output to RA0.
	DACCON0bits.DAC1OE = 1;
}

/**
 * Initializes the interrupt used to detect button presses.
 */
void InitializeButtonInterrupt(void) {
	// Unlock the PPS and set the Select button as a source for the INT interrupt.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;   // Unlock the PPS.
	INTPPSbits.INTPPS = 0b00011; // Set RA3 as an input to INT.
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1;   // Lock the PPS.
	INTCONbits.INTEDG = 0;       // Trigger on the falling edge.
	PIE0bits.INTE = 1;           // Enable the INT interrupt.
}

/**
 * Initializes the timer that will be used to determine a button hold.
 */
void InitializeButtonHoldTimer(void) {
	// Setup the Timer1 as a button hold timer of ~2s.
	T1CONbits.TMR1ON = 0;     // Disable the timer.
	T1CONbits.TMR1CS = 0b11;  // LFINTOSC as the clock source.
	T1CONbits.T1CKPS = 0b00;  // Prescaler set to 1.
	T1GCONbits.TMR1GE = 0;    // Disable gating. Will run like any other timer.
	PIE1bits.TMR1IE = 1;      // Enable its interrupt.
}

/**
 * Initializes the timer that will be used to flash the configuration lights
 * when configuring the charger.
 */
void InitializeFlashingTimer(void) {
	// Setup Timer6 as the timer for blinkenlights.
	T6CONbits.TMR6ON = 0;       // Disable the timer.
	T6CONbits.T6CKPS = 0b11;    // Prescaler set to 64.
	T6CONbits.T6OUTPS = 0b1111; // Postscaler set to 16.
	PR6 = 0xFF;                 // Period set to maximum.
	PIE2bits.TMR6IE = 1;        // Enable the timer interrupt.
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
