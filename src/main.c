#include <unistd.h>
#include <stdio.h>
#include "../include/proc_parser.h"
#include "../include/detector.h"
#include "../include/logger.h"

#define MAX_PIDS 32768

int main() {
    int pids[MAX_PIDS];
    ProcessInfo p_info;

    init_logger();
    printf("--- Process Sentinel Started ---\n");
    printf("Monitoring /proc for anomalies... (Ctrl+C to stop)\n");

    while (1) {
        int count = get_process_list(pids, MAX_PIDS);
        
        check_fork_bomb(count);

        for (int i = 0; i < count; i++) {
            if (read_process_stat(pids[i], &p_info)) {
                check_zombie(p_info);
                check_memory_hog(p_info);
            }
        }

        sleep(2); // Check every 2 seconds
    }
    return 0;
}