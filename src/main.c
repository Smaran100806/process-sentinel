#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/proc_parser.h"
#include "../include/detector.h"
#include "../include/logger.h"

volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    keep_running = 0;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    setsid();
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    umask(0);
    chdir("/");
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int is_daemon = (argc > 1 && strcmp(argv[1], "--daemon") == 0);
    if (is_daemon) daemonize();

    // Safety: Create logs directory if it doesn't exist
    mkdir("logs", 0777);

    int *pids = calloc(65536, sizeof(int));
    init_logger(); // Initialize logger AFTER daemonization

    ProcessInfo p_info;
    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;

    while (keep_running) {
        int count = get_process_list(pids, 65536);
        check_fork_bomb(count);

        if (!is_daemon) {
            printf("\033[2J\033[H"); 
            printf("\033[1;31m--- PROCESS SENTINEL: THREAT DASHBOARD ---\033[0m\n");
            printf("%-8s %-15s %-10s %-10s\n", "PID", "NAME", "RSS(MB)", "TYPE");
            printf("---------------------------------------------------\n");
        }

        for (int i = 0; i < count; i++) {
            if (read_process_stat(pids[i], &p_info)) {
                long rss_mb = (p_info.rss * page_size_kb) / 1024;
                
                // DASHBOARD: Shows Hogs (>500MB) or Zombies
                if (!is_daemon) {
                    if (p_info.state == 'Z' || rss_mb > 500) {
                        char *type = (p_info.state == 'Z') ? "\033[1;33mZOMBIE\033[0m" : "\033[1;31mMEM_HOG\033[0m";
                        printf("%-8d %-15s %-10ld %-10s\n", 
                               p_info.pid, p_info.name, rss_mb, type);
                    }
                }
                
                // DETECTOR: Checks for Spikes (>100MB new or >10MB growth)
                check_zombie(p_info);
                check_memory_hog(p_info);
            }
        }

        if (!is_daemon) printf("\nScanning %d procs... (Showing threats > 500MB)\n", count);
        sleep(2);
    }

    free(pids);
    close_logger();
    return 0;
}