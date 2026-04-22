
#ifndef CLOCKUPDATE_H_
#define CLOCKUPDATE_H_

#include <stdint.h>
#include "launchpad.h"
#include "seg7.h"

extern Seg7Display seg7;
extern uint32_t display_distance_mm;

void ClockUpdate(Event *event);

#endif /* CLOCKUPDATE_H_ */

