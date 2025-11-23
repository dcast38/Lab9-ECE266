/*
 * lab9_main.c: Starter code for ECE 266 Lab 9, main.c, fall 2024
 * Note: In this semester, a small part of starter code is provided
 * (none was provided before).
 *
 * Lab 8: Distance Measurement
 *
 * Created by David Castro
 * Last update: fall 2025
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "launchpad.h"
#include "ranger.h"
#include "buzzer.h"
#include "light.h"

#define TIMER_FREQ        50000000.0f  // 50 MHz
#define SOUND_SPEED       340.0f


/*
 * Global variables
 */


typedef struct
{
    int pitch;            // Current pitch level
    int volume;           // Current sound level
    bool buzzer_on;             // If buzzer is turned on or off
} SysState;

SysState sys = { 0, 0, false };

typedef struct
{
    bool light_on;             // If light is turned on or off
} SysState1;


// The events
Event trigger_ranger_reading_event;
Event push_button_event;
Event play_sound_event;


/*******************************************
 * Task 1: Trigger and read the ranger
 ******************************************/

// Trigger the ranger reading every 0.5 seconds

void TriggerRangerReading(Event *event){

    uint32_t pulse;
    float distance_mm;

    if(!RangerDataReady()){
        RangerTriggerReading();
    }
    else{
        pulse = RangerGetData();

        distance_mm = pulse * SOUND_SPEED / (2 * TIMER_FREQ ) * 1000.0;


        uprintf("Distance: %.2f mm%s\n\r", distance_mm);
    }
    EventSchedule(event, event->time + 500);
}


// read the distance so we can measure it for the buzzer
float read_distance_mm()
{
    static float last_distance = 1066.8f;   // default max distance
    uint32_t pulse;

    // Only read if ranger has a new measurement
    if (RangerDataReady()) {
        pulse = RangerGetData();

        float distance_mm = pulse * SOUND_SPEED / (2 * TIMER_FREQ) * 1000.0f;

        // clamp to max measurable range
        if (distance_mm > 1066.8f) {
            distance_mm = 1066.8f;
        }

        last_distance = distance_mm;     // save successfully read distance
    }

    return last_distance;
}


void PlaySound(Event *event){

    int delay;
    delay = 0;
    float d = read_distance_mm();

    if(d >= 1066.8f){   // far
        BuzzerOff();
        LightOff();
        sys.buzzer_on = false;
        delay = 300;
    }
    else if(d >= 800.0f){
        BuzzerToggle(800, 0.5f);
        sys.buzzer_on = true;
        delay = 250;
    }
    else if(d >= 600.0f){
        BuzzerToggle(1200, 0.5f);
        sys.buzzer_on = true;
        delay = 210;
    }
    else if(d >= 400.0f) {
        BuzzerToggle(1800, 0.5f);
        sys.buzzer_on = true;
        delay = 180;
    }
    else if(d >= 200.0f) {
        BuzzerToggle(2100, 0.5f);
        sys.buzzer_on = true;
        delay = 120;
    }
    else if (d >= 150.0f) {
        BuzzerToggle(2200, 0.5f);
        sys.buzzer_on = true;
        delay = 80;
    }
    else{
        BuzzerSet(2800, 0.5f);
        sys.buzzer_on = true;
    }
    if(delay < 20) delay = 20;
    EventSchedule(event, event->time + delay);
}

/*******************************************
 * The main function
 ******************************************/
void main(void)
{
    // Initialize the LaunchPad and peripherals
    LaunchPadInit();
    RangerInit();
    BuzzerInit();
    LightInit();


    // Initialize the events
    EventInit(&trigger_ranger_reading_event, TriggerRangerReading);
    EventInit(&play_sound_event, PlaySound);

    // Schedule time event
    EventSchedule(&trigger_ranger_reading_event, 100);
    EventSchedule(&play_sound_event, 100);


    uprintf("%s\n\r", "Lab 9");

    // Loop forever
    while (true)
    {
        // Wait for interrupt
        asm("   wfi");

        // Execute scheduled callbacks
        EventExecute();
    }
}
