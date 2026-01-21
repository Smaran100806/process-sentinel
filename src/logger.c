#include <stdio.h>
#include <time.h>
#include "../include/logger.h"

static const char *LOG_FILE = "logs/anomalies.log";

void init_logger() {
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp) {
        fprintf(fp, "--- SENTINEL DAEMON STARTED ---\n");
        fclose(fp);
    }
}

void log_alert(LogLevel level, const char *msg, int related_pid) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;

    time_t now = time(NULL);
    char *ts = ctime(&now);
    ts[24] = '\0'; // Remove newline

    const char *level_str = (level == LOG_DANGER) ? "DANGER" : 
                            (level == LOG_WARNING) ? "WARNING" : "INFO";

    fprintf(fp, "[%s] [%s] PID:%d | %s\n", ts, level_str, related_pid, msg);
    fclose(fp);
}