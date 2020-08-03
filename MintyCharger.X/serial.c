/**
 * serial.c
 * Handles serial communication.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "serial.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"

/**
 * Initializes the serial port.
 */
void InitializeSerial(void) {
	// Unlock the PPS and set the TX output pin.
	ANSELAbits.ANSA2 = 0;
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;    // Unlock the PPS.
	RA2PPSbits.RA2PPS = 0b10100;
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1;    // Lock the PPS.
	
	// Setup baud rate for 9600 baud.
	TX1STAbits.BRGH = 1;     // High speed serial.
	BAUD1CONbits.BRG16 = 1;  // 16-bit baud generator.
	SP1BRG = 832;            // 9600 @ 32MHz.
	TX1STAbits.SYNC = 0;     // Asynchronous operation.
	RC1STAbits.SPEN = 1;     // Enable the serial port.
	TX1STAbits.TX9 = 0;      // 8-bit transmission.
	TX1STAbits.TXEN = 1;     // Enable transmission.
}

/**
 * Sends a single character via serial.
 * 
 * @param c Character to be sent.
 */
inline void putc(const char c) {
	while (TX1STAbits.TRMT == 0);  // Wait for the transmit buffer to be empty.
	TX1REG = c;                    // Send the character.
}

/**
 * Sends a whole string via serial.
 * 
 * @param str String to be sent.
 */
void print(const char *str) {
	while (*str) {
		putc(*str++);
	}
}

/**
 * Sends s whole string together with a CRLF via serial.
 * 
 * @param str String to be sent.
 */
void println(const char *str) {
	print(str);
	putc('\r');
	putc('\n');
}
