#ifndef xf_timer_h
#define xf_timer_h

#include <stdint.h>
#include <time.h>

typedef struct _XF_Timer {
    struct timespec start;
    struct timespec end;

    double delta; // in ms
} XF_Timer;

void XF_StartTimer(XF_Timer* timer);
void XF_StopTimer(XF_Timer* timer);

#endif
