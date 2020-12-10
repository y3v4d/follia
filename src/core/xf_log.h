#ifndef xf_log_h
#define xf_log_h

#include "xf_utils.h"
#include <stdarg.h>

#define XF_LOG_ENABLED 0

enum XF_LogType {
    XF_LOG_INFO = 0,
    XF_LOG_WARNING = 1,
    XF_LOG_ERROR = 2
};

boolean XF_InitializeLog();
void XF_WriteLog(enum XF_LogType type, const char *format, ...);

#endif
