#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <cstdint>

// Mock Arduino types - ESP32 environment already provides these
// No need to redefine uint32_t, uint8_t, size_t, or boolean

// Additional Arduino types
typedef uint8_t byte;

// WiFi status constants
#define WL_CONNECTED 3
#define WL_DISCONNECTED 6

// Arduino constants
#define HEX 16

// Mock Arduino functions - declarations only
unsigned long millis();
void delay(unsigned long ms);
void yield();

// Mock String class for Arduino compatibility
class ArduinoString {
private:
    std::string data;
    
public:
    ArduinoString() : data("") {}
    ArduinoString(const char* str) : data(str ? str : "") {}
    ArduinoString(const std::string& str) : data(str) {}
    ArduinoString(int value) : data(std::to_string(value)) {}
    ArduinoString(unsigned long value) : data(std::to_string(value)) {}
    ArduinoString(int value, int base) : data(std::to_string(value)) {} // Simplified for testing
    
    const char* c_str() const { return data.c_str(); }
    int indexOf(const std::string& str) const { 
        size_t pos = data.find(str);
        return pos != std::string::npos ? static_cast<int>(pos) : -1;
    }
    int indexOf(const char* str) const { 
        size_t pos = data.find(str);
        return pos != std::string::npos ? static_cast<int>(pos) : -1;
    }
    
    // Add indexOf with start position
    int indexOf(const std::string& str, int fromIndex) const { 
        size_t pos = data.find(str, fromIndex);
        return pos != std::string::npos ? static_cast<int>(pos) : -1;
    }
    
    int indexOf(const char* str, int fromIndex) const { 
        size_t pos = data.find(str, fromIndex);
        return pos != std::string::npos ? static_cast<int>(pos) : -1;
    }
    
    bool operator==(const char* str) const { return data == str; }
    bool operator==(const std::string& str) const { return data == str; }
    bool operator==(const ArduinoString& str) const { return data == str.data; }
    
    // Add comparison operators for map support
    bool operator<(const ArduinoString& other) const { return data < other.data; }
    bool operator<(const char* str) const { return data < str; }
    bool operator<(const std::string& str) const { return data < str; }
    
    ArduinoString& operator+=(const ArduinoString& other) {
        data += other.data;
        return *this;
    }
    
    ArduinoString& operator+=(const char* str) {
        data += str;
        return *this;
    }
    
    ArduinoString& operator+=(const std::string& str) {
        data += str;
        return *this;
    }
    
    ArduinoString& operator+=(int value) {
        data += std::to_string(value);
        return *this;
    }
    
    ArduinoString& operator+=(unsigned long value) {
        data += std::to_string(value);
        return *this;
    }
    
    size_t length() const { return data.length(); }
    
    // Fix method ambiguity by removing the default parameter version
    ArduinoString substring(size_t from, size_t to) const {
        return ArduinoString(data.substr(from, to - from));
    }
    
    ArduinoString substring(size_t from) const {
        return ArduinoString(data.substr(from));
    }
    
    ArduinoString trim() {
        ArduinoString result = *this;
        result.data.erase(0, result.data.find_first_not_of(" \t\r\n"));
        result.data.erase(result.data.find_last_not_of(" \t\r\n") + 1);
        return result;
    }
    
    // Add missing String methods
    bool startsWith(const ArduinoString& str) const {
        return data.substr(0, str.length()) == str.data;
    }
    
    bool startsWith(const char* str) const {
        return data.substr(0, strlen(str)) == str;
    }
    
    bool endsWith(const ArduinoString& str) const {
        if (str.length() > data.length()) return false;
        return data.substr(data.length() - str.length()) == str.data;
    }
    
    bool endsWith(const char* str) const {
        size_t strLen = strlen(str);
        if (strLen > data.length()) return false;
        return data.substr(data.length() - strLen) == str;
    }
    
    char charAt(size_t index) const {
        if (index < data.length()) {
            return data[index];
        }
        return '\0';
    }
    
    int toInt() const {
        try {
            return std::stoi(data);
        } catch (...) {
            return 0;
        }
    }
    
    // Add operator+ for string concatenation
    ArduinoString operator+(const ArduinoString& other) const {
        ArduinoString result = *this;
        result += other;
        return result;
    }
    
    ArduinoString operator+(const char* str) const {
        ArduinoString result = *this;
        result += str;
        return result;
    }
    
    ArduinoString operator+(const std::string& str) const {
        ArduinoString result = *this;
        result += str;
        return result;
    }
    
    ArduinoString operator+(int value) const {
        ArduinoString result = *this;
        result += value;
        return result;
    }
    
    ArduinoString operator+(unsigned long value) const {
        ArduinoString result = *this;
        result += value;
        return result;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const ArduinoString& str) {
        return os << str.data;
    }
    
    // Friend operators for string concatenation with char arrays
    friend ArduinoString operator+(const char* lhs, const ArduinoString& rhs) {
        ArduinoString result(lhs);
        result += rhs;
        return result;
    }
    
    friend ArduinoString operator+(const std::string& lhs, const ArduinoString& rhs) {
        ArduinoString result(lhs);
        result += rhs;
        return result;
    }
    
    friend ArduinoString operator+(int lhs, const ArduinoString& rhs) {
        ArduinoString result(std::to_string(lhs));
        result += rhs;
        return result;
    }
    
    friend ArduinoString operator+(unsigned long lhs, const ArduinoString& rhs) {
        ArduinoString result(std::to_string(lhs));
        result += rhs;
        return result;
    }
};

// Use ArduinoString as String for testing
typedef ArduinoString String;

// Mock SPIFFS
class MockSPIFFS {
public:
    static bool begin(bool formatOnFail = false) { return true; }
    static bool exists(const char* path) { return true; }
    static bool exists(const String& path) { return true; }
    static bool remove(const char* path) { return true; }
    static bool remove(const String& path) { return true; }
    static bool format() { return true; }
};

// Mock File class
class MockFile {
private:
    std::string content;
    size_t position;
    
public:
    MockFile() : content(""), position(0) {}
    MockFile(const std::string& data) : content(data), position(0) {}
    
    bool seek(size_t pos) { 
        if (pos <= content.length()) {
            position = pos;
            return true;
        }
        return false;
    }
    
    size_t readBytes(char* buffer, size_t length) { 
        size_t available = content.length() - position;
        size_t toRead = (length < available) ? length : available;
        
        if (toRead > 0) {
            memcpy(buffer, content.c_str() + position, toRead);
            position += toRead;
        }
        
        return toRead;
    }
    
    size_t write(const uint8_t* buffer, size_t size) {
        content.append(reinterpret_cast<const char*>(buffer), size);
        return size;
    }
    
    size_t size() const { return content.length(); }
    void close() {}
    bool isDirectory() const { return false; }
    String name() const { return "mock_file"; }
    
    void setContent(const std::string& data) { content = data; }
    std::string getContent() const { return content; }
};

// Mock WiFi
class MockWiFi {
public:
    static bool begin(const char* ssid, const char* password) { return true; }
    static bool begin(const char* ssid, const char* password, int32_t channel, const uint8_t* bssid, bool connect) { return true; }
    static void disconnect(bool wifioff = false, bool eraseap = false) {}
    static bool isConnected() { return true; }
    static String SSID() { return "MockWiFi"; }
    static String localIP() { return "192.168.1.100"; }
    static String macAddress() { return "AA:BB:CC:DD:EE:FF"; }
    static int RSSI() { return -50; }
    static int scanNetworks() { return 5; }
    static String SSID(int i) { 
        ArduinoString result = "Network";
        result += i;
        return result;
    }
    static int RSSI(int i) { return -50 - (i * 10); }
    static int encryptionType(int i) { return 3; } // WPA2
};

// Mock Wire (I2C)
class MockWire {
public:
    static void begin() {}
    static void begin(int sda, int scl) {}
    static void beginTransmission(uint8_t address) {}
    static uint8_t endTransmission() { return 0; }
    static uint8_t requestFrom(uint8_t address, uint8_t quantity) { return quantity; }
    static size_t write(uint8_t data) { return 1; }
    static size_t write(const uint8_t* data, size_t quantity) { return quantity; }
    static int available() { return 1; }
    static int read() { return 0xAA; }
    static int readBytes(uint8_t* buffer, size_t length) { 
        for (size_t i = 0; i < length; i++) {
            buffer[i] = 0xAA + i;
        }
        return length;
    }
};

// Mock WebServer
class MockWebServer {
private:
    std::map<std::string, std::string> headers;
    std::string body;
    
public:
    void on(const char* uri, void (*handler)()) {}
    void on(const char* uri, const char* method, void (*handler)()) {}
    void send(int code, const char* contentType, const char* content) {}
    void send(int code, const char* contentType, const String& content) {}
    void sendHeader(const char* name, const char* value) {
        headers[name] = value;
    }
    void sendHeader(const char* name, const String& value) {
        headers[name] = value.c_str();
    }
    void setContentLength(size_t len) {}
    void enableCORS(bool enable = true) {}
    void enableCrossOrigin(bool enable = true) {}
    
    std::string getHeader(const char* name) const {
        auto it = headers.find(name);
        return (it != headers.end()) ? it->second : "";
    }
    
    void setBody(const std::string& data) { body = data; }
    std::string getBody() const { return body; }
};

// Mock HTTPUpload
class MockHTTPUpload {
public:
    String filename;
    String name;
    String contentType;
    size_t size;
    uint8_t* buf;
    
    MockHTTPUpload() : filename(""), name(""), contentType(""), size(0), buf(nullptr) {}
    
    void setData(const std::string& data) {
        size = data.length();
        if (buf) delete[] buf;
        buf = new uint8_t[size];
        memcpy(buf, data.c_str(), size);
    }
    
    ~MockHTTPUpload() {
        if (buf) delete[] buf;
    }
};

// Mock Serial
class MockSerial {
public:
    static void println(const String& message) {
        // Mock serial output - could be captured for testing if needed
    }
    
    static void println(const char* message) {
        // Mock serial output
    }
    
    static void print(const String& message) {
        // Mock serial output
    }
    
    static void print(const char* message) {
        // Mock serial output
    }
};

// Global mock instances
extern MockSPIFFS SPIFFS;
extern MockWiFi WiFi;
extern MockWire Wire;
extern MockWebServer server;
extern MockHTTPUpload upload;
extern MockSerial Serial;

#endif // MOCK_ARDUINO_H
