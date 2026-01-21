#include <string.h>

// Helper to remove newlines from strings (often needed when reading files)
void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}