#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "../include/detector.h"
#include "../include/logger.h"

static int prev_process_count = 0;

void check_fork_bomb(int current_proc_count) {
    if (prev_process_count > 0) {
        int diff = current_proc_count - prev_process_count;
        if (diff > 50) { 
            char msg[128];
            snprintf(msg, sizeof(msg), "Process spike: +%d procs (Possible Fork Bomb)", diff);
            log_alert(LOG_DANGER, msg, 0);
        }
    }
    prev_process_count = current_proc_count;
}

void check_zombie(ProcessInfo p) {
    if (p.state == 'Z') {
        char msg[128];
        snprintf(msg, sizeof(msg), "Zombie detected (Name: %s, Parent: %d)", p.name, p.ppid);
        log_alert(LOG_WARNING, msg, p.pid);
    }
}

void check_memory_hog(ProcessInfo p) {
    if (p.pid == getpid()) return;

    // Dynamically calculate page size
    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    long rss_mb = (p.rss * page_size_kb) / 1024;

    if (rss_mb > 100) { // Threshold: 100MB
        char msg[128];
        snprintf(msg, sizeof(msg), "High Memory: %ld MB (Process: %s)", rss_mb, p.name);
        log_alert(LOG_INFO, msg, p.pid);
    }
}