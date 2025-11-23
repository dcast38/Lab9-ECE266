/*
 * buzzer.h: Header file for buzzer-related functions
 *
 * Lab 4 starter code
 * ECE 266, spring 2025
 *
 * Created by Zhao Zhang
 *
 */

#ifndef LIGHT_H_
#define LIGHT_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include <driverlib/adc.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>

// Initialize the light
void LightInit();

// Set the light
uint32_t ReadLight();

// Turn off the light
void LightOff();

#endif /* LIGHT_H_ */
