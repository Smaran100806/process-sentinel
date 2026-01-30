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

int get_system_pid_max() {
    FILE *f = fopen("/proc/sys/kernel/pid_max", "r");
    int max_pids = 32768;
    if (f) {
        if (fscanf(f, "%d", &max_pids) != 1) max_pids = 32768;
        fclose(f);
    }
    return max_pids + 1024;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    if (chdir("/") < 0) exit(EXIT_FAILURE);

    // Redirect standard streams to /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) close(fd);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int is_daemon = (argc > 1 && strcmp(argv[1], "--daemon") == 0);
    if (is_daemon) daemonize();

    int max_pids = get_system_pid_max();
    int *pids = calloc(max_pids, sizeof(int));
    if (!pids) return 1;

    init_logger();
    if (!is_daemon) {
        printf("--- Process Sentinel Started (PID: %d) ---\n", getpid());
    }

    ProcessInfo p_info;
    while (keep_running) {
        int count = get_process_list(pids, max_pids);
        check_fork_bomb(count);

        for (int i = 0; i < count; i++) {
            if (read_process_stat(pids[i], &p_info)) {
                check_zombie(p_info);
                check_memory_hog(p_info);
            }
        }
        sleep(2);
    }

    free(pids);
    close_logger();
    return 0;
}