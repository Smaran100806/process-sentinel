#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../include/detector.h"
#include "../include/logger.h"

static int prev_process_count = 0;

void check_fork_bomb(int current_proc_count) {
    if (prev_process_count > 0) {
        int diff = current_proc_count - prev_process_count;
        if (diff > 50) { // Threshold: 50+ processes in one cycle
            char msg[128];
            snprintf(msg, sizeof(msg), "Process spike detected: +%d processes (Possible Fork Bomb)", diff);
            log_alert(LOG_DANGER, msg, 0);
        }
    }
    prev_process_count = current_proc_count;
}

void check_zombie(ProcessInfo p) {
    if (p.state == 'Z') {
        char msg[128];
        snprintf(msg, sizeof(msg), "Zombie process detected (Parent PID: %d)", p.ppid);
        log_alert(LOG_WARNING, msg, p.pid);
    }
}

void check_memory_hog(ProcessInfo p) {
    // Self-protection: Ignore the sentinel itself
    if (p.pid == getpid()) return;

    // RSS is in pages (typically 4KB). 
    // Threshold: > 100MB (~25600 pages)
    if (p.rss > 25600) {
        char msg[128];
        snprintf(msg, sizeof(msg), "High Memory Usage detected: %ld pages", p.rss);
        log_alert(LOG_INFO, msg, p.pid);
    }
}