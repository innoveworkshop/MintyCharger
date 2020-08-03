/**
 * serial.h
 * Handles serial communication.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef SERIAL_H
#define	SERIAL_H

#include <stdint.h>

// Initialization.
void InitializeSerial(void);

// Sending data.
extern inline void __attribute__((always_inline)) putc(const char c);
void print(const char *str);
void println(const char *str);
void printi(const int i);

#endif	/* SERIAL_H */

