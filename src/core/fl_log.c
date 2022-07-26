#include "core/fl_log.h"

#include <stdio.h>
#include <time.h>
#include <errno.h>

FILE *log_file = NULL;
char path[64];
FL_Bool initialized = false;

FL_Bool FL_InitializeLog() {
    if(!FL_LOG_ENABLED) return true;

    if(initialized) {
        printf("[WARNING] Log file is already opened");
        return true;
    }

    time_t raw_time;
    struct tm *loc_time;

    time(&raw_time);
    loc_time = localtime(&raw_time);

    strftime(path, 64, "build/logs/log_%C%m%d-%H%M%S.txt", loc_time); 
    
    log_file = fopen(path, "w");
    if(!log_file) {
        fprintf(stderr, "[ERROR] Couldn't open log file");
        return false;
    }

    initialized = true;

    fclose(log_file);

    return true;
}

void FL_WriteLog(FL_LogType type, const char *format, ...) {
    if(!FL_LOG_ENABLED) return;

    if(!initialized) return;

    log_file = fopen(path, "a");
    if(!log_file) {
        fprintf(stderr, "[ERROR] Couldn't write to log file");
        return;
    }

    switch(type) {
        case FL_LOG_INFO: fprintf(log_file, "[INFO] "); break;
        case FL_LOG_WARNING: fprintf(log_file, "[WARNING] "); break;
        case FL_LOG_ERROR: fprintf(log_file, "[ERROR] "); break;
        default: break;
    }

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    if(type == FL_LOG_ERROR) {
        va_start(args, format);
        fprintf(stderr, "[ERROR] ");
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
        va_end(args);
    }

    fclose(log_file);
}
