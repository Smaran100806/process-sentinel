#include <stdio.h>
#include "../include/detector.h"
#include "../include/logger.h"

static int prev_process_count = 0;

void check_fork_bomb(int current_proc_count) {
    if (prev_process_count > 0) {
        int diff = current_proc_count - prev_process_count;
        if (diff > 50) { // If 50+ processes spawned in 2 seconds
            char msg[100];
            snprintf(msg, sizeof(msg), "Process spike detected: +%d processes (Possible Fork Bomb)", diff);
            log_alert(LOG_DANGER, msg, 0);
        }
    }
    prev_process_count = current_proc_count;
}

void check_zombie(ProcessInfo p) {
    if (p.state == 'Z') {
        char msg[100];
        snprintf(msg, sizeof(msg), "Zombie process detected (Parent PID: %d)", p.ppid);
        log_alert(LOG_WARNING, msg, p.pid);
    }
}

void check_memory_hog(ProcessInfo p) {
    // RSS is in pages (usually 4KB). Let's warn if > 100MB (~25000 pages)
    if (p.rss > 25000) {
        char msg[100];
        snprintf(msg, sizeof(msg), "High Memory Usage detected: %ld pages", p.rss);
        log_alert(LOG_INFO, msg, p.pid);
    }
}