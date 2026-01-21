#ifndef LOGGER_H
#define LOGGER_H

// Severity levels
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_DANGER
} LogLevel;

void init_logger();
void close_logger(); // Added cleanup function
void log_alert(LogLevel level, const char *msg, int related_pid);

#endif