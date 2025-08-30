#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger {
public:
    static const int MAX_LOG_ENTRIES = 100;
    
    static void init();
    static void addEntry(String message);
    static String getLogs();
    static void clearLogs();
    static int getLogCount();
    
    // New method for getting log entries as plain text
    static String getLogEntries();

private:
    static String logEntries[MAX_LOG_ENTRIES];
    static int logIndex;
    static bool logWrapped;
};

#endif
