#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <SPIFFS.h>
#include <Wire.h>
#include "Logger.h"
#include "LEDController.h"
#include "I2CScanner.h"
#include "FirmwareUpdater.h"
#include "HomeAssistantMQTT.h"
#include "ConfigManager.h"
#include "WebHandler.h"

// WiFi and Web Server
WebServer server(80);
WiFiManager wifiManager;

// Function prototypes
void setupWebServer();

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    // Initialize all systems
    Logger::init();
    ConfigManager::init();
    LEDController::init();
    I2CScanner::init();
    FirmwareUpdater::init();
    HomeAssistantMQTT::init();
    
    // Startup sequence
    Logger::addEntry("ESP32 C3 Mini 1 Starting...");
    LEDController::startupSequence();
    
    // Initialize SPIFFS for storing WiFi credentials and serving HTML files
    if (!SPIFFS.begin(true)) {
        Logger::addEntry("SPIFFS initialization failed");
    }
    
    // Configure WiFi Manager
    wifiManager.setConfigPortalTimeout(180); // 3 minutes timeout
    wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
        Logger::addEntry("Entered WiFi config mode");
        Logger::addEntry("AP IP: " + WiFi.softAPIP().toString());
        Logger::addEntry("SSID: " + myWiFiManager->getConfigPortalSSID());
        LEDController::wifiConfigMode();
    });
    
    // Save WiFi credentials when successfully connected
    wifiManager.setSaveConfigCallback([]() {
        String ssid = WiFi.SSID();
        String password = WiFi.psk();
        Logger::addEntry("WiFi credentials saved via WiFiManager: " + ssid);
        ConfigManager::setWiFiConfig(ssid, password);
    });
    
    // Try to connect to saved WiFi
    Logger::addEntry("Attempting to connect to saved WiFi...");
    
    // Check if we have stored WiFi credentials
    WiFiConfig storedWiFi = ConfigManager::getWiFiConfig();
    if (storedWiFi.ssid.length() > 0) {
        Logger::addEntry("Attempting to connect to stored WiFi: " + storedWiFi.ssid);
        WiFi.begin(storedWiFi.ssid.c_str(), storedWiFi.password.c_str());
        
        // Wait for connection with timeout
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Logger::addEntry("Connected to stored WiFi successfully!");
            // Ensure credentials are saved even when connecting to stored WiFi
            String ssid = WiFi.SSID();
            String password = WiFi.psk();
            if (ssid.length() > 0) {
                ConfigManager::setWiFiConfig(ssid, password);
                Logger::addEntry("WiFi credentials confirmed and saved: " + ssid);
            }
        } else {
            Logger::addEntry("Failed to connect to stored WiFi, entering setup mode");
            if (!wifiManager.autoConnect("ESP32C3_Setup")) {
                Logger::addEntry("Failed to connect and hit timeout");
                LEDController::wifiFailed();
                ESP.restart();
            }
        }
    } else {
        Logger::addEntry("No stored WiFi credentials, entering setup mode");
        if (!wifiManager.autoConnect("ESP32C3_Setup")) {
            Logger::addEntry("Failed to connect and hit timeout");
            LEDController::wifiFailed();
            ESP.restart();
        }
    }
    
    // Connected to WiFi
    Logger::addEntry("Connected to WiFi successfully!");
    Logger::addEntry("IP Address: " + WiFi.localIP().toString());
    Logger::addEntry("MAC Address: " + WiFi.macAddress());
    Logger::addEntry("Signal Strength: " + String(WiFi.RSSI()) + " dBm");
    
    // Ensure current WiFi credentials are saved
    String currentSSID = WiFi.SSID();
    String currentPassword = WiFi.psk();
    if (currentSSID.length() > 0) {
        ConfigManager::setWiFiConfig(currentSSID, currentPassword);
        Logger::addEntry("Current WiFi credentials saved: " + currentSSID);
    }
    
    LEDController::wifiConnected();
    
    // Connect to Home Assistant via MQTT
    HomeAssistantMQTT::connect();
    
    // Set up MQTT message callback to handle commands from Home Assistant
    HomeAssistantMQTT::setMessageCallback([](const String& topic, const String& payload) {
        // Handle LED control commands from Home Assistant
        if (topic.endsWith("/led_control/set")) {
            if (payload == "ON") {
                LEDController::setColorByName("white"); // Default to white when turned on
                HomeAssistantMQTT::publishLEDState("white");
            } else if (payload == "OFF") {
                LEDController::setColorByName("off");
                HomeAssistantMQTT::publishLEDState("off");
            }
        }
    });
    
    // Scan I2C bus
    Logger::addEntry("Scanning I2C bus...");
    I2CScanner::scan();
    
    // Setup web server
    setupWebServer();
    
    // Start web server
    server.begin();
    Logger::addEntry("Web server started!");
    
    // Periodic tasks
    Logger::addEntry("System initialization complete");
}

void loop() {
    server.handleClient();
    
    // Handle MQTT operations
    if (HomeAssistantMQTT::isConnected()) {
        // MQTT client handles its own loop internally
    }
    
    // Update uptime in log and publish to Home Assistant every 30 seconds
    static unsigned long lastUptimeLog = 0;
    if (millis() - lastUptimeLog > 30000) {
        lastUptimeLog = millis();
        String uptime = String(millis() / 1000);
        int rssi = WiFi.RSSI();
        
        Logger::addEntry("Uptime: " + uptime + "s, WiFi RSSI: " + String(rssi) + " dBm");
        
        // Publish to Home Assistant if MQTT is connected
        if (HomeAssistantMQTT::isConnected()) {
            HomeAssistantMQTT::publishSystemStatus(uptime, rssi);
        }
    }
    
    delay(10);
}

void setupWebServer() {
    // Initialize WebHandler with the server instance
    WebHandler::init(&server);
}