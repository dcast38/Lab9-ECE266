/*
 * lab9_main.c: Starter code for ECE 266 Lab 9, main.c, fall 2024
 * Note: In this semester, a small part of starter code is provided
 * (none was provided before).
 *
 * Lab 9: Distance Measurement
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
#include "seg7.h"
#include "clock.h"
#define TIMER_FREQ        50000000.0f  // 50 MHz
#define SOUND_SPEED       340.0f


/*
 * Global variables
 */

typedef struct
{
    bool red;     // red sub-LED on/off
    bool blue;    // blue sub-LED on/off
    bool green;   // green sub-LED on/off
} LEDColorSetting;
#define NUM_DISTANCE_BANDS 7
static const LEDColorSetting distance_led_colors[NUM_DISTANCE_BANDS] =
{
    // case 0: (d >= 1066.8)
    { false, false, false }, // off

    // case 1: 800 < d < 1066.8
    { false, true,  false },   // blue

    // case 2: 600 < d < 800
    { false, true,  true  },   // cyan

    // case 3: 400 < d < 600
    { false, false, true  },   // green

    // case 4: 200 < d < 400
    { true,  false, true  },   // yellow

    // case 5: 150 <= d < 200
    { true,  false, true  },   // orange

    // case 6: d < 150
    { true,  false, false }    // red
};

static void SetLEDFromBand(int band)
{
    if (band < 0) band = 0;
    if (band >= NUM_DISTANCE_BANDS) band = NUM_DISTANCE_BANDS - 1;

    LEDColorSetting c = distance_led_colors[band];
    LedTurnOnOff(c.red, c.blue, c.green);
}


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
float read_distance()
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

static int DistanceInCases(float d)
{
    if (d >= 1066.8f)      return 0;
    else if (d >= 800.0f)  return 1;
    else if (d >= 600.0f)  return 2;
    else if (d >= 400.0f)  return 3;
    else if (d >= 200.0f)  return 4;
    else if (d >= 150.0f)  return 5;
    else                   return 6;
}

void PlaySound(Event *event)
{
    int delay = 0;
    float d = read_distance_mm();

    int band = DistanceInCases(d);
    SetLEDFromBand(band);

    switch (band)
    {
    case 0:
        BuzzerOff();
        sys.buzzer_on = false;
        delay = 300;
        break;

    case 1:
        BuzzerToggle(800, 0.5f);
        sys.buzzer_on = true;
        delay = 250;
        break;

    case 2:
        BuzzerToggle(1200, 0.5f);
        sys.buzzer_on = true;
        delay = 210;
        break;

    case 3:
        BuzzerToggle(1800, 0.5f);
        sys.buzzer_on = true;
        delay = 180;
        break;

    case 4:
        BuzzerToggle(2100, 0.5f);
        sys.buzzer_on = true;
        delay = 120;
        break;

    case 5:
        BuzzerToggle(2200, 0.5f);
        sys.buzzer_on = true;
        delay = 80;
        break;

    case 6:
    default:
        BuzzerSet(2800, 0.5f);
        sys.buzzer_on = true;
        break;
    }
    if (delay < 20)
        delay = 20;

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
    Event seg7_event;
    Seg7Init();
    Seg7Update(&seg7);


    // Initialize the event
    EventInit(&trigger_ranger_reading_event, TriggerRangerReading);
    EventInit(&play_sound_event, PlaySound);
    EventInit(&seg7_event, 

    // Schedule time event
    EventSchedule(&trigger_ranger_reading_event, 100);
    EventSchedule(&play_sound_event, 100);
    EventSchedule(&seg7_event, 100);
    

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


