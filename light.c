

#include "light.h"

/******************************************************************************
 * Initialize the buzzer
 * Pin usage: Grove base port J17, Tiva C PC5 (Port C, Pin 5)
 *****************************************************************************/
void LightInit()
{
    // Enable Port E peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

        // Configure ADC0 sequencer 3 (single sample)
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE, 3);
}
uint32_t ReadLight(){
    uint32_t value;
    ADCProcessorTrigger(ADC0_BASE, 3);

            // Wait for conversion to complete
            while(!ADCIntStatus(ADC0_BASE, 3, false));

            // Read ADC value
            ADCSequenceDataGet(ADC0_BASE, 3, &value);

            // Clear interrupt
            ADCIntClear(ADC0_BASE, 3);

            return value;
}
void LightOff(){
}
