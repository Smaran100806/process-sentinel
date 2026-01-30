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
    if (!fp) return 0;
    
    char buffer[4096]; 
    if (!fgets(buffer, sizeof(buffer), fp)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // Find the bounds of the process name (between first '(' and last ')')
    char *left_paren = strchr(buffer, '(');
    char *right_paren = strrchr(buffer, ')');
    
    if (!left_paren || !right_paren || left_paren > right_paren) return 0;

    // Extract PID
    p_info->pid = atoi(buffer);

    // Extract Name safely
    int name_len = right_paren - (left_paren + 1);
    if (name_len >= sizeof(p_info->name)) name_len = sizeof(p_info->name) - 1;
    strncpy(p_info->name, left_paren + 1, name_len);
    p_info->name[name_len] = '\0';

    // Parse stats from after the last ')'
    char *rest = right_paren + 2;
    if (sscanf(rest, "%c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld",
           &p_info->state,
           &p_info->ppid,
           &p_info->utime,
           &p_info->stime,
           &p_info->rss) < 5) {
        return 0;
    }

    return 1;
}