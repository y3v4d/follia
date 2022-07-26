#ifndef __FL_LOG_H__
#define __FL_LOG_H__

#include "fl_utils.h"
#include <stdarg.h>

#define FL_LOG_ENABLED 1 // set it to 1 if you want to get log files

typedef enum {
    FL_LOG_INFO = 0,
    FL_LOG_WARNING = 1,
    FL_LOG_ERROR = 2
} FL_LogType;

// User don't have to invoke that function if the FL_LOG_ENABLED is set to 1.
FL_Bool FL_InitializeLog();

/*
 * FL_WriteLog
 * Can be used to write custom information to the log file, and won't work if FL_LOG_ENABLED is set to 0.
 * Works like printf function, but the first argument is the type of the message.
 */
void FL_WriteLog(FL_LogType type, const char *format, ...);

#endif
