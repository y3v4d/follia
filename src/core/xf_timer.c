#include "core/xf_timer.h"
#include "core/xf_log.h"

void XF_StartTimer(XF_Timer* timer) {
    if(!timer) {
        XF_WriteLog(XF_LOG_ERROR, "Invalid timer passed to XF_StartTimer!\n");
        return;
    }

    clock_gettime(CLOCK_REALTIME, &timer->start);
}

void XF_StopTimer(XF_Timer* timer) {
    if(!timer) {
        XF_WriteLog(XF_LOG_ERROR, "Invalid timer passed to XF_EndTimer!\n");
        return;
    }

    clock_gettime(CLOCK_REALTIME, &timer->end);

    struct timespec delta;
    delta.tv_sec = timer->end.tv_sec - timer->start.tv_sec;
    delta.tv_nsec = timer->end.tv_nsec - timer->start.tv_nsec;

    if(delta.tv_nsec < 0) {
        delta.tv_nsec += 1000000000;
        --delta.tv_sec;
    }

    timer->delta = delta.tv_sec * 1000 + (double)delta.tv_nsec / 1000000;
}