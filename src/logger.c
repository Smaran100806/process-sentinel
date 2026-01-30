#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "../include/logger.h"

static const char *LOG_FILE = "logs/anomalies.log";
static FILE *log_fp = NULL;

void init_logger() {
    if (log_fp != NULL) return;
    log_fp = fopen(LOG_FILE, "a");
    if (log_fp) {
        fprintf(log_fp, "\n--- SENTINEL DAEMON STARTED ---\n");
        fflush(log_fp);
    }
}

void close_logger() {
    if (log_fp) {
        fprintf(log_fp, "--- SENTINEL DAEMON STOPPED ---\n");
        fclose(log_fp);
        log_fp = NULL;
    }
}

void log_alert(LogLevel level, const char *msg, int related_pid) {
    if (!log_fp) return;

    time_t now = time(NULL);
    char *ts = ctime(&now);
    if (ts) ts[24] = '\0'; 

    const char *level_str = (level == LOG_DANGER) ? "DANGER" : 
                            (level == LOG_WARNING) ? "WARNING" : "INFO";

    fprintf(log_fp, "[%s] [%s] PID:%d | %s\n", ts ? ts : "TIME_ERR", level_str, related_pid, msg);
    fflush(log_fp); 
}