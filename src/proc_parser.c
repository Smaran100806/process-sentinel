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
    if (!fp) return 0; // Process died
    
    char buffer[2048]; // Large buffer to read the whole line
    if (!fgets(buffer, sizeof(buffer), fp)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // Robust parsing: Find the *last* closing parenthesis ')'
    // This handles cases like "process (1) (helper)" correctly
    char *right_paren = strrchr(buffer, ')');
    if (!right_paren) return 0;

    // Everything before the last ')' is related to PID and Name
    *right_paren = '\0'; // Split the string temporarily

    // Parse PID and Name
    // The name starts after the first '(', but simple sscanf works on the first part
    sscanf(buffer, "%d (%s", &p_info->pid, p_info->name);
    
    // Restore the name (remove the leading parenthesis if sscanf didn't catch it cleanly)
    // A safer manual extraction for name:
    char *left_paren = strchr(buffer, '(');
    if (left_paren) {
        strncpy(p_info->name, left_paren + 1, sizeof(p_info->name) - 1);
        p_info->name[sizeof(p_info->name) - 1] = '\0';
    }

    // Parse stats: The rest of the string starts 2 chars after the ')'
    // (space + state character)
    // Structure of /proc/[pid]/stat:
    // 1 (name) S 2 ...
    // buffer points to "1 (name", right_paren+2 points to "S 2 ..."
    
    char *rest = right_paren + 2;
    sscanf(rest, "%c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld",
           &p_info->state,
           &p_info->ppid,
           &p_info->utime,
           &p_info->stime,
           &p_info->rss);

    return 1;
}