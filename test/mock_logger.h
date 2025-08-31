#ifndef MOCK_LOGGER_H
#define MOCK_LOGGER_H

#include "mock_arduino.h"

class Logger {
public:
    static const int MAX_LOG_ENTRIES = 100;
    
    static void init();
    static void addEntry(String message);
    static String getLogs();
    static void clearLogs();
    static int getLogCount();
    static String getLogEntries();

private:
    static String logEntries[MAX_LOG_ENTRIES];
    static int logIndex;
    static bool logWrapped;
};

#endif // MOCK_LOGGER_H
