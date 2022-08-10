#ifndef __FL_TIMER_H__
#define __FL_TIMER_H__

#include <stdint.h>
#include <time.h>

#define FL_TIMER_CLEAR_SCREEN 0x00
#define FL_TIMER_RENDER 0x01
#define FL_TIMER_CLEAR_TO_RENDER 0x02
#define FL_TIMER_ALL 0x03

typedef struct {
    struct timespec start;
    struct timespec end;

    double delta; // in ms
} FL_Timer;

void FL_StartTimer(FL_Timer* timer);
void FL_StopTimer(FL_Timer* timer);

#endif
