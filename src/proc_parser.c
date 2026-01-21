#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include "../include/proc_parser.h"

int get_process_list(int *pid_list, int max_size) {
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    
    int count = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (isdigit(dir->d_name[0])) {
                if (count < max_size) {
                    pid_list[count++] = atoi(dir->d_name);
                }
            }
        }
        closedir(d);
    }
    return count;
}

int read_process_stat(int pid, ProcessInfo *p_info) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) return 0; // Process died before we could read it
    
    // Parse the stat file based on Linux documentation
    // Note: The process name is in parentheses (name)
    fscanf(fp, "%d %s %c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld",
           &p_info->pid,
           p_info->name,
           &p_info->state,
           &p_info->ppid,
           &p_info->utime,
           &p_info->stime,
           &p_info->rss);
           
    fclose(fp);
    return 1;
}