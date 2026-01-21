#ifndef PROC_PARSER_H
#define PROC_PARSER_H

typedef struct {
    int pid;
    char name[256];
    char state;          // 'R' (Running), 'S' (Sleep), 'Z' (Zombie)
    unsigned long utime; // CPU time in user mode
    unsigned long stime; // CPU time in kernel mode
    long rss;            // RAM usage (Resident Set Size)
    int ppid;            // Parent Process ID
} ProcessInfo;

int get_process_list(int *pid_list, int max_size);
int read_process_stat(int pid, ProcessInfo *p_info);

#endif