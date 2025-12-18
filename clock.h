#ifndef CLOCK_H_
#define CLOCK_H_

#include "launchpad.h"
#include "seg7.h"

// Declare global variable seg7
extern Seg7Display seg7;
extern uint32_t display_distance_mm;

// Display the current band name on the 7-segment display
void ClockUpdate(Event *event);

#endif /* CLOCK_H_ */

