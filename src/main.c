#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/proc_parser.h"
#include "../include/detector.h"
#include "../include/logger.h"

// Global flag for signal handling
volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    keep_running = 0;
}

// Function to determine max PIDs dynamically
int get_system_pid_max() {
    FILE *f = fopen("/proc/sys/kernel/pid_max", "r");
    int max_pids = 32768; // Default fallback
    if (f) {
        fscanf(f, "%d", &max_pids);
        fclose(f);
    }
    return max_pids + 1024; // buffer
}

void daemonize() {
    pid_t pid;

    // Fork off the parent process
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Parent exits

    // On success: The child process becomes session leader
    if (setsid() < 0) exit(EXIT_FAILURE);

    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // Fork off for the second time
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    // Set new file permissions
    umask(0);

    // Change the working directory to the root directory
    chdir("/");

    // Close all open file descriptors
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }
}

int main(int argc, char *argv[]) {
    // Setup signal handling for clean exit
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Optional: Pass "--daemon" to run in background
    if (argc > 1 && strcmp(argv[1], "--daemon") == 0) {
        daemonize();
    }

    // Dynamic memory allocation based on system limits
    int max_pids = get_system_pid_max();
    int *pids = malloc(max_pids * sizeof(int));
    if (!pids) {
        perror("Failed to allocate memory for PID list");
        return 1;
    }

    ProcessInfo p_info;

    init_logger();
    
    // If not a daemon, print to console too
    if (argc <= 1 || strcmp(argv[1], "--daemon") != 0) {
        printf("--- Process Sentinel Started ---\n");
        printf("Monitoring /proc for anomalies... (Ctrl+C to stop)\n");
    }

    while (keep_running) {
        int count = get_process_list(pids, max_pids);
        
        check_fork_bomb(count);

        for (int i = 0; i < count; i++) {
            if (read_process_stat(pids[i], &p_info)) {
                check_zombie(p_info);
                check_memory_hog(p_info);
            }
        }

        sleep(2); // Check every 2 seconds
    }

    // Cleanup
    free(pids);
    close_logger();
    return 0;
}