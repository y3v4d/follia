#include "core/fl_timer.h"
#include "core/fl_log.h"

void FL_StartTimer(FL_Timer* timer) {
    if(!timer) {
        FL_WriteLog(FL_LOG_ERROR, "Invalid timer passed to FL_StartTimer!\n");
        return;
    }

    clock_gettime(CLOCK_REALTIME, &timer->start);
}

void FL_StopTimer(FL_Timer* timer) {
    if(!timer) {
        FL_WriteLog(FL_LOG_ERROR, "Invalid timer passed to FL_EndTimer!\n");
        return;
    }

    clock_gettime(CLOCK_REALTIME, &timer->end);

    timer->delta = (timer->end.tv_sec - timer->start.tv_sec) * 1e3 + (timer->end.tv_nsec - timer->start.tv_nsec) / 1e6;
}
