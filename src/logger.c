#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "../include/logger.h"

static const char *LOG_FILE = "logs/anomalies.log";
static FILE *log_fp = NULL;

void init_logger() {
    if (log_fp != NULL) return; // Already initialized

    log_fp = fopen(LOG_FILE, "a");
    if (log_fp) {
        fprintf(log_fp, "--- SENTINEL DAEMON STARTED ---\n");
        fflush(log_fp);
    } else {
        perror("Failed to open log file");
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
    if (ts) ts[24] = '\0'; // Remove newline

    const char *level_str = (level == LOG_DANGER) ? "DANGER" : 
                            (level == LOG_WARNING) ? "WARNING" : "INFO";

    fprintf(log_fp, "[%s] [%s] PID:%d | %s\n", ts ? ts : "UNKNOWN TIME", level_str, related_pid, msg);
    fflush(log_fp); // Ensure data is written to disk immediately
}