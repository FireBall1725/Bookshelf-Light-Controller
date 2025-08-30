#include "WebHandler.h"
#include <WiFi.h>
#include <WebServer.h>

// Static member initialization
WebServer* WebHandler::webServer = nullptr;

void WebHandler::init(WebServer* server) {
    webServer = server;
    setupRoutes();
}

void WebHandler::setupRoutes() {
    if (!webServer) return;
    
    // Static file handlers
    webServer->on("/", HTTP_GET, handleRoot);
    webServer->on("/styles.css", HTTP_GET, handleCSS);
    webServer->on("/script.js", HTTP_GET, handleJavaScript);
    webServer->on("/config", HTTP_GET, handleConfigPage);
    webServer->on("/wifi", HTTP_GET, handleWiFiPage);
    
    // API endpoints
    webServer->on("/led", HTTP_GET, handleLED);
    webServer->on("/uptime", HTTP_GET, handleUptime);
    webServer->on("/log", HTTP_GET, handleLog);
    webServer->on("/clearlog", HTTP_GET, handleClearLog);
    webServer->on("/scani2c", HTTP_GET, handleScanI2C);
    webServer->on("/i2ccmd", HTTP_GET, handleI2CCommand);
    webServer->on("/versioncheck", HTTP_GET, handleVersionCheck);
    webServer->on("/firmwareupload", HTTP_POST, []() {
        // Handle the POST request completion
        webServer->send(200, "text/plain", "Upload completed");
    }, handleFirmwareUpload);
    webServer->on("/firmwareupdate", HTTP_GET, handleFirmwareUpdate);
    
    // Configuration endpoints
    webServer->on("/config", HTTP_POST, handleConfigUpdate);
    webServer->on("/wifi", HTTP_POST, handleWiFiUpdate);
    
    // Firmware management endpoints
    webServer->on("/firmware/list", HTTP_GET, handleFirmwareList);
    webServer->on("/firmware/info", HTTP_GET, handleFirmwareInfo);
    webServer->on("/firmware/delete", HTTP_GET, handleFirmwareDelete);
    
    // Firmware package management endpoints
    webServer->on("/firmware/packages", HTTP_GET, handleFirmwarePackages);
    webServer->on("/firmware/package/info", HTTP_GET, handleFirmwarePackageInfo);
    webServer->on("/firmware/package/delete", HTTP_GET, handleFirmwarePackageDelete);
    webServer->on("/firmware/all", HTTP_GET, handleAllFirmware);
    
    // API endpoints for configuration
    webServer->on("/api/config", HTTP_GET, handleAPIConfig);
    webServer->on("/api/wifi", HTTP_GET, handleAPIWiFi);
}

// Static file handlers
void WebHandler::handleRoot() {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
        webServer->send(404, "text/plain", "File not found");
        return;
    }
    webServer->streamFile(file, "text/html");
    file.close();
}

void WebHandler::handleCSS() {
    File file = SPIFFS.open("/styles.css", "r");
    if (!file) {
        webServer->send(404, "text/plain", "File not found");
        return;
    }
    webServer->streamFile(file, "text/css");
    file.close();
}

void WebHandler::handleJavaScript() {
    File file = SPIFFS.open("/script.js", "r");
    if (!file) {
        webServer->send(404, "text/plain", "File not found");
        return;
    }
    webServer->streamFile(file, "application/javascript");
    file.close();
}

void WebHandler::handleConfigPage() {
    File file = SPIFFS.open("/config.html", "r");
    if (!file) {
        webServer->send(404, "text/plain", "File not found");
        return;
    }
    webServer->streamFile(file, "text/html");
    file.close();
}

void WebHandler::handleWiFiPage() {
    File file = SPIFFS.open("/wifi.html", "r");
    if (!file) {
        webServer->send(404, "text/plain", "File not found");
        return;
    }
    webServer->streamFile(file, "text/html");
    file.close();
}

// API endpoints
void WebHandler::handleLED() {
    if (webServer->hasArg("colour")) {
        String colour = webServer->arg("colour");
        LEDController::setColorByName(colour);
        webServer->send(200, "text/plain", "LED set to " + colour);
    } else {
        webServer->send(400, "text/plain", "Missing colour parameter");
    }
}

void WebHandler::handleUptime() {
    String response = "{\"uptime\":\"" + getUptimeString() + "\",\"mac\":\"" + getMACAddress() + "\",\"ip\":\"" + getIPAddress() + "\",\"rssi\":" + String(getWiFiRSSI()) + "}";
    webServer->send(200, "application/json", response);
}

void WebHandler::handleLog() {
    String logEntries = Logger::getLogEntries();
    webServer->send(200, "text/plain", logEntries);
}

void WebHandler::handleClearLog() {
    Logger::clearLogs();
    webServer->send(200, "text/plain", "Log cleared");
}

void WebHandler::handleScanI2C() {
    String devices = I2CScanner::scan();
    webServer->send(200, "text/plain", devices);
}

void WebHandler::handleI2CCommand() {
    if (webServer->hasArg("cmd")) {
        int command = webServer->arg("cmd").toInt();
        I2CScanner::sendCommand(command);
        webServer->send(200, "text/plain", "I2C command sent: 0x" + String(command, HEX));
    } else {
        webServer->send(400, "text/plain", "Missing command parameter");
    }
}

void WebHandler::handleVersionCheck() {
    Logger::addEntry("Checking ATtiny1616 version...");
    
    if (FirmwareUpdater::checkATtinyVersion()) {
        webServer->send(200, "text/plain", "Version check completed successfully. Check logs for version details.");
    } else {
        webServer->send(500, "text/plain", "Failed to read version");
    }
}

void WebHandler::handleFirmwareUpload() {
    // Custom file upload handler for large files
    // The ESP32 WebServer has limited buffer size, so we implement our own
    
    // Get the uploaded file data
    HTTPUpload& upload = webServer->upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Logger::addEntry("Firmware upload started: " + upload.filename);
        
        // Create a temporary file in SPIFFS
        String tempPath = "/temp_" + upload.filename;
        File tempFile = SPIFFS.open(tempPath, "w");
        if (!tempFile) {
            Logger::addEntry("Failed to create temporary file: " + tempPath);
            webServer->send(500, "text/plain", "Server error: cannot create temporary file");
            return;
        }
        tempFile.close();
        
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // Write chunk to temporary file
        String tempPath = "/temp_" + upload.filename;
        File tempFile = SPIFFS.open(tempPath, "a");
        if (tempFile) {
            size_t bytesWritten = tempFile.write(upload.buf, upload.currentSize);
            tempFile.close();
            
            if (bytesWritten != upload.currentSize) {
                Logger::addEntry("Failed to write chunk: expected " + String(upload.currentSize) + ", wrote " + String(bytesWritten));
                return;
            }
            
            Logger::addEntry("Wrote chunk: " + String(upload.currentSize) + " bytes");
        } else {
            Logger::addEntry("Failed to open temporary file for writing");
            return;
        }
        
    } else if (upload.status == UPLOAD_FILE_END) {
        Logger::addEntry("Firmware upload completed, size: " + String(upload.totalSize) + " bytes");
        
        // Read the complete file from SPIFFS
        String tempPath = "/temp_" + upload.filename;
        if (!SPIFFS.exists(tempPath)) {
            Logger::addEntry("Temporary file not found: " + tempPath);
            webServer->send(400, "text/plain", "Upload failed - temporary file not found");
            return;
        }
        
        File tempFile = SPIFFS.open(tempPath, "r");
        if (!tempFile) {
            Logger::addEntry("Failed to open temporary file: " + tempPath);
            webServer->send(400, "text/plain", "Upload failed - cannot read temporary file");
            return;
        }
        
        size_t fileSize = tempFile.size();
        Logger::addEntry("File size from SPIFFS: " + String(fileSize) + " bytes");
        
        if (fileSize != upload.totalSize) {
            Logger::addEntry("Size mismatch: expected " + String(upload.totalSize) + ", got " + String(fileSize));
            tempFile.close();
            SPIFFS.remove(tempPath);
            webServer->send(400, "text/plain", "Upload failed - size mismatch");
            return;
        }
        
        // Read the entire file into memory
        uint8_t* firmwareBytes = new uint8_t[fileSize];
        size_t bytesRead = tempFile.read(firmwareBytes, fileSize);
        tempFile.close();
        
        if (bytesRead != fileSize) {
            Logger::addEntry("Failed to read complete file. Expected: " + String(fileSize) + ", Read: " + String(bytesRead));
            delete[] firmwareBytes;
            SPIFFS.remove(tempPath);
            webServer->send(400, "text/plain", "Upload failed - incomplete read");
            return;
        }
        
        // Debug: Log the first 8 bytes from file
        String debugBytes = "First 8 bytes from file: ";
        for (int i = 0; i < 8 && i < fileSize; i++) {
            debugBytes += String(firmwareBytes[i], HEX) + " ";
        }
        Logger::addEntry(debugBytes);
        
        // Determine if this is a .bin package or .hex file
        String filename = upload.filename;
        bool isPackage = filename.endsWith(".bin");
        
        bool success = false;
        if (isPackage) {
            // Handle as firmware package (.bin file)
            Logger::addEntry("Processing firmware package: " + filename);
            success = FirmwareUpdater::uploadFirmwarePackage(firmwareBytes, fileSize, filename);
            if (success) {
                Logger::addEntry("Firmware package uploaded and extracted successfully: " + String(fileSize) + " bytes");
                webServer->send(200, "text/plain", "Firmware package uploaded and extracted successfully! Size: " + String(fileSize) + " bytes");
            } else {
                Logger::addEntry("Failed to upload firmware package");
                webServer->send(400, "text/plain", "Failed to upload firmware package");
            }
        } else {
            // Handle as legacy .hex file
            Logger::addEntry("Processing legacy hex file: " + filename);
            success = FirmwareUpdater::uploadFirmwareToSPIFFS(firmwareBytes, fileSize, "attiny_firmware.hex");
            if (success) {
                Logger::addEntry("Firmware uploaded to SPIFFS successfully: " + String(fileSize) + " bytes");
                webServer->send(200, "text/plain", "Firmware uploaded to SPIFFS successfully! Size: " + String(fileSize) + " bytes");
            } else {
                Logger::addEntry("Failed to upload firmware to SPIFFS");
                webServer->send(400, "text/plain", "Failed to upload firmware to SPIFFS");
            }
        }
        
        delete[] firmwareBytes; // Clean up memory
        
        // Clean up the temporary file
        SPIFFS.remove(tempPath);
        
    } else {
        Logger::addEntry("Firmware upload error: " + String(upload.status));
        webServer->send(400, "text/plain", "Firmware upload error: " + String(upload.status));
    }
}

void WebHandler::handleFirmwareUpdate() {
    Logger::addEntry("Starting ATtiny1616 firmware update...");
    
    if (FirmwareUpdater::updateATtinyFirmware()) {
        webServer->send(200, "text/plain", "ATtiny1616 firmware update completed successfully!");
    } else {
        webServer->send(500, "text/plain", "Firmware update failed. Check logs for details.");
    }
}

// Configuration endpoints
void WebHandler::handleConfig() {
    // This is now handled by handleConfigPage() for GET requests
}

void WebHandler::handleConfigUpdate() {
    if (webServer->hasArg("brokerIP") && webServer->hasArg("brokerPort")) {
        String brokerIP = webServer->arg("brokerIP");
        int brokerPort = webServer->arg("brokerPort").toInt();
        String username = webServer->hasArg("username") ? webServer->arg("username") : "";
        String password = webServer->hasArg("password") ? webServer->arg("password") : "";
        String deviceName = webServer->hasArg("deviceName") ? webServer->arg("deviceName") : "ESP32_C3_Mini";
        String deviceId = webServer->hasArg("deviceId") ? webServer->arg("deviceId") : "esp32_c3_mini_1";
        String mqttPrefix = webServer->hasArg("mqttPrefix") ? webServer->arg("mqttPrefix") : "homeassistant";
        
        if (ConfigManager::updateMQTTConfig(brokerIP, brokerPort, username, password, deviceName, deviceId, mqttPrefix)) {
            Logger::addEntry("MQTT configuration updated, reconnecting...");
            webServer->send(200, "text/plain", "MQTT configuration updated successfully! Device will reconnect with new settings.");
        } else {
            webServer->send(500, "text/plain", "Failed to update MQTT configuration.");
        }
    } else {
        webServer->send(400, "text/plain", "Missing required parameters.");
    }
}

void WebHandler::handleWiFiConfig() {
    // This is now handled by handleWiFiPage() for GET requests
}

void WebHandler::handleWiFiUpdate() {
    if (webServer->hasArg("ssid") && webServer->hasArg("password")) {
        String ssid = webServer->arg("ssid");
        String password = webServer->arg("password");
        
        ConfigManager::setWiFiConfig(ssid, password);
        
        webServer->send(200, "text/plain", "WiFi credentials updated! Device will restart to apply new settings.");
        
        // Restart the device after a short delay
        delay(2000);
        ESP.restart();
    } else {
        webServer->send(400, "text/plain", "Missing SSID or password.");
    }
}

// Firmware management endpoints
void WebHandler::handleFirmwareList() {
    String firmwareList = FirmwareUpdater::listStoredFirmwares();
    webServer->send(200, "text/plain", firmwareList);
}

void WebHandler::handleFirmwareInfo() {
    if (webServer->hasArg("filename")) {
        String filename = webServer->arg("filename");
        String info = FirmwareUpdater::getStoredFirmwareInfo(filename);
        webServer->send(200, "text/plain", info);
    } else {
        // Default to main firmware file
        String info = FirmwareUpdater::getStoredFirmwareInfo();
        webServer->send(200, "text/plain", info);
    }
}

void WebHandler::handleFirmwareDelete() {
    if (webServer->hasArg("filename")) {
        String filename = webServer->arg("filename");
        if (FirmwareUpdater::deleteStoredFirmware(filename)) {
            webServer->send(200, "text/plain", "Firmware file deleted: " + filename);
        } else {
            webServer->send(500, "text/plain", "Failed to delete firmware file: " + filename);
        }
    } else {
        webServer->send(400, "text/plain", "Missing filename parameter");
    }
}

// API endpoints for configuration
void WebHandler::handleAPIConfig() {
    MQTTConfig config = ConfigManager::getMQTTConfig();
    
    String json = "{";
    json += "\"brokerIP\":\"" + config.brokerIP + "\",";
    json += "\"brokerPort\":" + String(config.brokerPort) + ",";
    json += "\"username\":\"" + config.username + "\",";
    json += "\"password\":\"" + config.password + "\",";
    json += "\"deviceName\":\"" + config.deviceName + "\",";
    json += "\"deviceId\":\"" + config.deviceId + "\",";
    json += "\"mqttPrefix\":\"" + config.mqttPrefix + "\"";
    json += "}";
    
    webServer->send(200, "application/json", json);
}

void WebHandler::handleAPIWiFi() {
    WiFiConfig wifi = ConfigManager::getWiFiConfig();
    
    String json = "{";
    json += "\"ssid\":\"" + wifi.ssid + "\",";
    json += "\"password\":\"" + wifi.password + "\"";
    json += "}";
    
    webServer->send(200, "application/json", json);
}

// Private helper methods
String WebHandler::getUptimeString() {
    unsigned long uptime = millis();
    unsigned long days = uptime / 86400000;
    uptime %= 86400000;
    unsigned long hours = uptime / 3600000;
    uptime %= 3600000;
    unsigned long minutes = uptime / 60000;
    uptime %= 60000;
    unsigned long seconds = uptime / 1000;
    
    String result = "";
    if (days > 0) result += String(days) + "d ";
    if (hours > 0 || days > 0) result += String(hours) + "h ";
    if (minutes > 0 || hours > 0 || days > 0) result += String(minutes) + "m ";
    result += String(seconds) + "s";
    
    return result;
}

String WebHandler::getMACAddress() {
    return WiFi.macAddress();
}

String WebHandler::getIPAddress() {
    return WiFi.localIP().toString();
}

int WebHandler::getWiFiRSSI() {
    return WiFi.RSSI();
}

// Firmware package management endpoints
void WebHandler::handleFirmwarePackages() {
    String packageList = FirmwareUpdater::listFirmwarePackages();
    webServer->send(200, "text/plain", packageList);
}

void WebHandler::handleFirmwarePackageInfo() {
    if (webServer->hasArg("filename")) {
        String filename = webServer->arg("filename");
        String info = FirmwareUpdater::getFirmwarePackageInfo(filename);
        webServer->send(200, "text/plain", info);
    } else {
        // Default to main package file
        String info = FirmwareUpdater::getFirmwarePackageInfo();
        webServer->send(200, "text/plain", info);
    }
}

void WebHandler::handleFirmwarePackageDelete() {
    if (webServer->hasArg("filename")) {
        String filename = webServer->arg("filename");
        if (FirmwareUpdater::deleteFirmwarePackage(filename)) {
            webServer->send(200, "text/plain", "Firmware package deleted: " + filename);
        } else {
            webServer->send(500, "text/plain", "Failed to delete firmware package: " + filename);
        }
    } else {
        webServer->send(400, "text/plain", "Missing filename parameter");
    }
}

void WebHandler::handleAllFirmware() {
    String allFirmwareInfo = FirmwareUpdater::getAllFirmwareInfo();
    webServer->send(200, "text/plain", allFirmwareInfo);
}
