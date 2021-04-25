#include "core/xf_timer.h"
#include "core/xf_log.h"

void XF_StartTimer(XF_Timer* timer) {
    if(!timer) {
        XF_WriteLog(XF_LOG_ERROR, "Invalid timer passed to XF_StartTimer!\n");
        return;
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timer->start);
}

void XF_StopTimer(XF_Timer* timer) {
    if(!timer) {
        XF_WriteLog(XF_LOG_ERROR, "Invalid timer passed to XF_EndTimer!\n");
        return;
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timer->end);

    timer->delta = (timer->end.tv_sec - timer->start.tv_sec) * 1e3 + (timer->end.tv_nsec - timer->start.tv_nsec) / 1e6;
}
