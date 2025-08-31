#include "mock_logger.h"

// Mock Logger implementation for testing
String Logger::logEntries[MAX_LOG_ENTRIES];
int Logger::logIndex = 0;
bool Logger::logWrapped = false;

void Logger::init() {
    logIndex = 0;
    logWrapped = false;
}

void Logger::addEntry(String message) {
    String timestamp = "[" + String(millis() / 1000) + "s]";
    String logEntry = timestamp + " " + message;
    
    logEntries[logIndex] = logEntry;
    logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
    
    if (logIndex == 0) {
        logWrapped = true;
    }
    
    // Mock serial output
    Serial.println(logEntry);
}

String Logger::getLogs() {
    String logHtml = "";
    
    if (logWrapped) {
        // Show entries from current index to end, then from beginning to current index
        for (int i = logIndex; i < MAX_LOG_ENTRIES; i++) {
            logHtml += "<div class='log-entry'>" + logEntries[i] + "</div>";
        }
        for (int i = 0; i < logIndex; i++) {
            logHtml += "<div class='log-entry'>" + logEntries[i] + "</div>";
        }
    } else {
        // Show entries from beginning to current index
        for (int i = 0; i < logIndex; i++) {
            logHtml += "<div class='log-entry'>" + logEntries[i] + "</div>";
        }
    }
    
    if (logHtml == "") {
        logHtml = "<div class='log-entry'>No log entries yet</div>";
    }
    
    return logHtml;
}

void Logger::clearLogs() {
    logIndex = 0;
    logWrapped = false;
    addEntry("Log cleared");
}

int Logger::getLogCount() {
    if (logWrapped) {
        return MAX_LOG_ENTRIES;
    }
    return logIndex;
}

String Logger::getLogEntries() {
    String logText = "";
    
    if (logWrapped) {
        // Show entries from current index to end, then from beginning to current index
        for (int i = logIndex; i < MAX_LOG_ENTRIES; i++) {
            logText += logEntries[i] + "\n";
        }
        for (int i = 0; i < logIndex; i++) {
            logText += logEntries[i] + "\n";
        }
    } else {
        // Show entries from beginning to current index
        for (int i = 0; i < logIndex; i++) {
            logText += logEntries[i] + "\n";
        }
    }
    
    if (logText == "") {
        logText = "No log entries yet\n";
    }
    
    return logText;
}
