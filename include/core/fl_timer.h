#ifndef __FL_TIMER_H__
#define __FL_TIMER_H__

#include <stdint.h>
#include <time.h>

typedef struct {
    struct timespec start;
    struct timespec end;

    double delta; // in ms
} FL_Timer;

void FL_StartTimer(FL_Timer* timer);
void FL_StopTimer(FL_Timer* timer);

#endif
