#ifndef xf_log_h
#define xf_log_h

#include "xf_utils.h"
#include <stdarg.h>

#define XF_LOG_ENABLED 1 // set it to 1 if you want to get log files

enum XF_LogType {
    XF_LOG_INFO = 0,
    XF_LOG_WARNING = 1,
    XF_LOG_ERROR = 2
};

// User don't have to invoke that function if the XF_LOG_ENABLED is set to 1.
boolean XF_InitializeLog();

/*
 * XF_WriteLog
 * Can be used to write custom information to the log file, and won't work if XF_LOG_ENABLED is set to 0.
 * Works like printf function, but the first argument is the type of the message.
 */
void XF_WriteLog(enum XF_LogType type, const char *format, ...);

#endif
