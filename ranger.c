/*
 * range.c: Starter code for ECE 266 Lab 8, main.c, fall 2025
 *
 * Lab 8: Distance Measurement
 *
 * YOU NEED TO REVISE THE CODE IN PART 2 AND PART 3.
 *
 * Created by Zhao Zhang
 * Last update: fall 2024
 */

#include "ranger.h"

/*
 * Hardware configure: The ultrasonic ranger (and its signal pin) shall be connected to:
 *      Grove baseboard: Port J6, Pin 24
 *      Tiva C: GPIO PD1
 *      Timer pin: WT2CCP1
 */

/*
 * Global/static variables and declarations
 */
typedef struct {
    Event *callback_event;
    enum {
        IDLE,                   // ranger is idle
        EXPECT_START_EVENT_1,   // expecting the rising edge of the start pulse
        EXPECT_START_EVENT_2,   // expecting the falling edge of the start pulse
        EXPECT_DATA_EVENT_1,    // expecting the rising edge of the data pulse
        EXPECT_DATA_EVENT_2     // expecting the falling edge of the data pulse
    } state;
    uint32_t rising_edge;
    uint32_t falling_edge;
    uint32_t pulse_width;
    bool new_data_ready;
} RangerState;

volatile RangerState ranger;

void RangerISR();

/*
 * Initialize the ranger
 */
void RangerInit()
{
    // Initial ranger state in memory
    ranger.callback_event = NULL;
    ranger.new_data_ready = false;

    // Enable GPIO Port D and Wide Timer 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2);

    // Configure PD2 as timer pin and connect it to WT2CCP1
    // Do NOT enable the time as this time
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD1_WT2CCP1);

    // Count both edges as event
    TimerControlLevel(WTIMER2_BASE, TIMER_B, true);
    TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);

    // Register an ISR to deal with the timeout event
    TimerIntRegister(WTIMER2_BASE, TIMER_B, RangerISR);
    TimerIntEnable(WTIMER2_BASE, TIMER_CAPB_EVENT);
}

/*
 * Send the start pulse to the ultrasonic ranger. The pin being used will be switched
 * to timer input pin at the end.
 */
void RangerTriggerReading()
{
    // Clean data ready flag and set the ranger state
    ranger.new_data_ready = false;
    ranger.state = EXPECT_START_EVENT_1;

    // It is safe to disable a peripheral during configuration
    TimerDisable(WTIMER2_BASE, TIMER_B);

    // Configure WT2CCP1 for PWM to generate a pulse of 5 microseconds, with
    // 2 microseconds of leading low time
    TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM);
    TimerLoadSet(WTIMER2_BASE, TIMER_B, 1000);
    TimerMatchSet(WTIMER2_BASE, TIMER_B, 500);
    TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);

    // Enable the timer, and start PWM waveform
    TimerEnable(WTIMER2_BASE, TIMER_B);
}

/*
 * If any new data is ready (for polling)
 */
bool RangerDataReady()
{
    return ranger.new_data_ready;
}


void RangerCapture(){
    TimerDisable(WTIMER2_BASE, TIMER_B);
    TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME);
    TimerLoadSet(WTIMER2_BASE, TIMER_B, 0xFFFFFFFF);
    TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
    TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
    TimerEnable(WTIMER2_BASE, TIMER_B);
}
/*
void RangerCapture()
{
    TimerDisable(WTIMER2_BASE, TIMER_B);

    // Switch PD1 back to timer capture function (critical!)
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_1);

    TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME);

    // Reset timer so rising edge is clean
    TimerLoadSet(WTIMER2_BASE, TIMER_B, 0xFFFFFFFF);
    TimerValueSet(WTIMER2_BASE, TIMER_B, 0xFFFFFFFF);

    TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);

    TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
    TimerEnable(WTIMER2_BASE, TIMER_B);
}
*/
/*
 * RangerISR: Process the events in the start pulse (from Tiva C to ranger)
 * and the data pulse (from ranger to Tiva C). It implements a finite 
 * state machine in software.
 *
 * There are four events to process:
 * 1. Expecting the rising edge of the start pulse
 * 2. Expecting the falling edge of the start pulse
 * 3. Expecting the rising edge of the data pulse
 * 4. Expecting the falling edge of the data pulse
 *
 * You ISR needs to process each event approximately.
 */
void RangerISR()
{
    TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
    switch (ranger.state)
    {
    case EXPECT_START_EVENT_1:  // case 1 rising edge
        ranger.state = EXPECT_START_EVENT_2; // set ranger state to falling edge
        break;

    case EXPECT_START_EVENT_2:   // case 2 falling edge
        RangerCapture();
        ranger.state = EXPECT_DATA_EVENT_1;  // set ranger state = 3 aka rising edge data pulse
        break;

    case EXPECT_DATA_EVENT_1:   //case 3 rising edge
        ranger.rising_edge = TimerValueGet(WTIMER2_BASE, TIMER_B);
        ranger.state = EXPECT_DATA_EVENT_2;  //set ranger state = 4 aka falling edge data pulse
        break;

    case EXPECT_DATA_EVENT_2:   //case 4 falling edge
        ranger.falling_edge = TimerValueGet(WTIMER2_BASE, TIMER_B);
        ranger.pulse_width = ranger.rising_edge - ranger.falling_edge; // or vice versa, depending on timer direction
        ranger.new_data_ready = true;
        ranger.state = IDLE;
        break;
    }

    TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
}

uint32_t RangerGetData(){

    ranger.new_data_ready = false;
    return ranger.pulse_width;

}
