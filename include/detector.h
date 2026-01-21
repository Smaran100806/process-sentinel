#ifndef DETECTOR_H
#define DETECTOR_H

#include "proc_parser.h"

void check_fork_bomb(int current_proc_count);
void check_zombie(ProcessInfo p);
void check_memory_hog(ProcessInfo p);

#endif