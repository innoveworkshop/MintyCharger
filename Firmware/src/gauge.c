/* 
 * gauge.c
 * Shows the user the current state of charge of the battery.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "gauge.h"
#include "config.h"
#include <xc.h>
#include <pic16f18325.h>
#include <stdbool.h>
#include <stdint.h>
#include "pins.h"
#include "interface.h"
#include "vreg.h"

/**
 * Updates the gauge display.
 */
void DisplayBatteryGauge(void) {
    uint8_t gauge = 0b0000;
    float voltage = GetCellVoltage();
    
    if (IsLithiumBattery()) {
        // 25%
        if (voltage > 3.75f)
            gauge |= 0b0001;
        
        // 50%
        if (voltage > 3.85f)
            gauge |= 0b0010;
        
        // 75%
        if (voltage > 4.00f)
            gauge |= 0b0100;
        
        // 100%
        if (IsFinishedCharging())
            gauge |= 0b1000;
    } else {
        // 25%
        if (voltage > 1.20f)
            gauge |= 0b0001;
        
        // 50%
        if (voltage > 1.35f)
            gauge |= 0b0010;
        
        // 75%
        if (voltage > 1.42f)
            gauge |= 0b0100;
        
        // 100%
        if (IsFinishedCharging())
            gauge |= 0b1000;
    }
    
    // Shift gauge to start at RC2 and push changes to the IO pins.
    gauge <<= 2;
    gauge += 0b11;
    LATC &= gauge;
}