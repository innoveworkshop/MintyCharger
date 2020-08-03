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

// Private methods.
char* itoa(const int value);

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

/**
 * Sends an integer as a string via serial.
 * 
 * @param i Integer value to be sent as string.
 */
void printi(const int i) {
	print(itoa(i));
}

/**
 * Converts an integer value into a string.
 * @see https://www.microchip.com/forums/FindPost/482866
 * 
 * @param  value Integer to be converted.
 * @return       String representation of the integer value.
 */
char* itoa(const int value) {
	static char buffer[12];  // 12-bytes is big enough for an INT32.
	int original = value;    // Save the original value.
	int val = value;
	int c = sizeof(buffer) - 1;
	buffer[c] = 0;           // Write null terminator to the last string byte.
	
	// If it's negative, note that and take the absolute value.
	if (val < 0)
		val = -val;

	// Write least significant digit of value that's left.
	do {
		buffer[--c] = (val % 10) + '0';
		val /= 10;
	} while (val);

	if (original < 0)
		buffer[--c] = '-';

	return &buffer[c];
}
