#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include "ConfigManager.h"
#include "FirmwareUpdater.h"
#include "Logger.h"
#include "LEDController.h"
#include "I2CScanner.h"

class WebHandler {
public:
    static void init(WebServer* server);
    static void setupRoutes();
    
    // Static file handlers
    static void handleRoot();
    static void handleCSS();
    static void handleJavaScript();
    static void handleConfigPage();
    static void handleWiFiPage();
    
    // API endpoints
    static void handleLED();
    static void handleUptime();
    static void handleLog();
    static void handleClearLog();
    static void handleScanI2C();
    static void handleI2CCommand();
    static void handleVersionCheck();
    static void handleFirmwareUpload();
    static void handleFirmwareUpdate();
    
    // Configuration endpoints
    static void handleConfig();
    static void handleConfigUpdate();
    static void handleWiFiConfig();
    static void handleWiFiUpdate();
    
    // Firmware management endpoints
    static void handleFirmwareList();
    static void handleFirmwareInfo();
    static void handleFirmwareDelete();
    
    // Firmware package management endpoints
    static void handleFirmwarePackages();
    static void handleFirmwarePackageInfo();
    static void handleFirmwarePackageDelete();
    static void handleAllFirmware();
    
    // API endpoints for configuration
    static void handleAPIConfig();
    static void handleAPIWiFi();

private:
    static WebServer* webServer;
    static String getUptimeString();
    static String getMACAddress();
    static String getIPAddress();
    static int getWiFiRSSI();
};

#endif
