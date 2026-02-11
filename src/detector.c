#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "../include/detector.h"
#include "../include/logger.h"

static int prev_process_count = 0;
static long total_system_rss_avg = 0;
static int observation_count = 0;
static int scan_cycles = 0; // Track how many times we've scanned the system

// Track history for ALL processes
static long previous_rss[65536] = {0}; 
static int last_logged_zombie[65536] = {0};

void check_fork_bomb(int current_proc_count) {
    if (prev_process_count > 0) {
        int diff = current_proc_count - prev_process_count;
        if (diff > 50) { 
            char msg[512]; 
            snprintf(msg, sizeof(msg), "Process spike: +%d (Possible Fork Bomb)", diff);
            log_alert(LOG_DANGER, msg, 0);
        }
    }
    prev_process_count = current_proc_count;
    scan_cycles++; // Increment cycle count
}

void check_zombie(ProcessInfo p) {
    if (p.state == 'Z' && p.pid < 65536) {
        if (last_logged_zombie[p.pid] == 0) {
            char msg[512];
            snprintf(msg, sizeof(msg), "ZOMBIE DETECTED: %s (Parent: %d)", p.name, p.ppid);
            log_alert(LOG_WARNING, msg, p.pid);
            last_logged_zombie[p.pid] = 1;
        }
    } else if (p.pid < 65536) {
        last_logged_zombie[p.pid] = 0;
    }
}

void check_memory_hog(ProcessInfo p) {
    if (p.pid == getpid() || p.pid >= 65536) return;

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    long rss_mb = (p.rss * page_size_kb) / 1024;

    // 1. Calculate Average
    if (observation_count < 10) {
        total_system_rss_avg += rss_mb;
        observation_count++;
    }
    long avg = (observation_count > 0) ? (total_system_rss_avg / observation_count) : 0;
    
    long hog_threshold = (avg * 5 > 500) ? (avg * 5) : 500;
    long spike_threshold = 100; 

    // 2. DETECTION LOGIC
    if (rss_mb > spike_threshold) {
        // CASE A: First time seeing this process
        if (previous_rss[p.pid] == 0) {
             // WARM-UP CHECK: Only log if we have finished at least 1 full scan
             if (scan_cycles > 1) { 
                 char msg[512];
                 snprintf(msg, sizeof(msg), "NEW HIGH MEM PROCESS: %s started with %ld MB", p.name, rss_mb);
                 log_alert(LOG_INFO, msg, p.pid);
             }
        }
        // CASE B: Existing process grew significantly (> 10MB)
        else if ((rss_mb - previous_rss[p.pid]) > 10) {
             char msg[512];
             snprintf(msg, sizeof(msg), "MEMORY SPIKE: %s grew by %ld MB (Total: %ld MB)", p.name, (rss_mb - previous_rss[p.pid]), rss_mb);
             log_alert(LOG_WARNING, msg, p.pid);
        }
    }

    // 3. PERSISTENT HOG LOGGING
    if (rss_mb > hog_threshold) {
         if (previous_rss[p.pid] > 0 && abs(rss_mb - previous_rss[p.pid]) > 10) {
             char msg[512];
             snprintf(msg, sizeof(msg), "HIGH MEMORY: %s using %ld MB (Baseline: %ld MB)", p.name, rss_mb, avg);
             log_alert(LOG_INFO, msg, p.pid);
         }
    }

    // Update history
    previous_rss[p.pid] = rss_mb;
}