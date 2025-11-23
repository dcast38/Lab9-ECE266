
#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/pwm.h>
#include <driverlib/pin_map.h>
#include "buzzer.h"

static bool buzzer_state = false;

void BuzzerInit(void)
{
    // Enable PWM0 and GPIOC peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));

    // Configure PC5 as PWM output pin
    GPIOPinConfigure(GPIO_PC5_M0PWM7);
    GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_5);

    // Configure PWM generator 3 (for output 7)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}

void BuzzerSet(uint32_t frequency, float duty_cycle){
    if(frequency == 0)
        {
            PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, false);
            return;
        }

        uint32_t period = SysCtlClockGet() / frequency;
        uint32_t pulse_width = (uint32_t)(period * duty_cycle);

        PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, period);
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, pulse_width);
        PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, true);
}

void BuzzerToggle(uint32_t frequency, float duty_cycle){
    if(buzzer_state) {
        PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, false);
        buzzer_state = false;
    }
    else {
        BuzzerSet(frequency, duty_cycle);
        buzzer_state = true;
    }
}

void BuzzerOff(){
    PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, false);
}

