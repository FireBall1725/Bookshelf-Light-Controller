# ESP32 C3 Mini 1 Controller & ATtiny1616 Firmware Updater

A comprehensive ESP32 C3 Mini 1 application with modular architecture for LED control, I2C communication, and ATtiny1616 firmware updates over I2C.

## 🏗️ Project Structure

```
ESP32 Test/
├── src/
│   └── main.cpp              # Main application (simplified, ~200 lines)
├── include/
│   ├── Logger.h              # Logging system header
│   ├── LEDController.h       # LED control header
│   ├── I2CScanner.h          # I2C scanning header
│   └── FirmwareUpdater.h     # ATtiny1616 firmware update header
├── lib/
│   ├── Logger/
│   │   └── Logger.cpp        # Logging system implementation
│   ├── LEDController/
│   │   └── LEDController.cpp # LED control implementation
│   ├── I2CScanner/
│   │   └── I2CScanner.cpp    # I2C scanning implementation
│   └── FirmwareUpdater/
│       └── FirmwareUpdater.cpp # Firmware update implementation
├── data/                     # Web interface files (served from SPIFFS)
│   ├── index.html            # Main HTML page
│   ├── styles.css            # CSS styling
│   └── script.js             # JavaScript functionality
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

## ✨ Features

### 🎮 LED Control
- Full RGB LED control with FastLED library
- 8 basic colours + special effects (rainbow, blink, fade)
- Visual status indicators for WiFi states

### 🔌 I2C Communication
- I2C bus scanning with device identification
- Command interface for address 0x50 devices
- Support for common I2C sensors and displays

### 📡 WiFi Management
- WiFiManager for easy WiFi setup
- Captive portal for credential input
- Persistent credential storage

### 🔄 ATtiny1616 Firmware Updates
- Complete firmware update protocol support
- Version checking and metadata extraction
- 64-byte chunk transmission with checksums
- Progress tracking and error handling

### 🌐 Web Interface
- Modern, responsive design
- Real-time system monitoring
- Interactive controls for all features
- Live log viewing and management

## 🚀 Local Development

### HTML/CSS/JS Development

The web interface is now completely separate from the C++ code, making it easy to develop and test locally:

1. **Open the `data/` folder** in your preferred web editor
2. **Edit HTML/CSS/JS files** directly
3. **Test in browser** - open `data/index.html` locally
4. **Upload to ESP32** - files are automatically served from SPIFFS

### File Structure Benefits

- **`index.html`** - Clean, semantic HTML structure
- **`styles.css`** - Modern CSS with gradients, animations, responsive design
- **`script.js`** - Modular JavaScript with error handling and notifications

### Local Testing

```bash
# Open index.html in your browser
open data/index.html

# Or serve with a local web server
cd data
python3 -m http.server 8000
# Then visit http://localhost:8000
```

## 🔧 Building and Uploading

### Prerequisites
- PlatformIO Core installed
- ESP32 C3 Mini 1 connected via USB

### Build Commands
```bash
# Build only
pio run

# Build and upload
pio run --target upload

# Build and upload with monitor
pio run --target upload --target monitor
```

### SPIFFS Upload
```bash
# Upload web interface files to SPIFFS
pio run --target uploadfs
```

## 📋 API Endpoints

### LED Control
- `GET /led?colour=<color>` - Set LED colour

### I2C Operations
- `GET /scani2c` - Scan I2C bus
- `GET /i2ccmd?cmd=<command>` - Send I2C command to 0x50

### Firmware Updates
- `GET /versioncheck` - Check ATtiny1616 version
- `POST /firmwareupload` - Upload .hex file
- `GET /firmwareupdate` - Start firmware update

### System Info
- `GET /uptime` - Get system uptime
- `GET /log` - Get system logs
- `GET /clearlog` - Clear system logs

## 🎯 ATtiny1616 Protocol

The system implements the complete ATtiny1616 firmware update protocol:

1. **Version Check** (0x10) - Read current firmware version
2. **Enter Update Mode** (0xFF) - Device starts red blinking
3. **Send Firmware** (0xFE) - 64-byte chunks with metadata
4. **Verify Update** (0xFD) - Send final checksum for validation
5. **Complete Update** (0xFC) - Device reboots with new firmware

## 🔍 Troubleshooting

### Common Issues

1. **Linter Errors** - Expected when using external libraries
2. **SPIFFS Issues** - Ensure `data/` folder is uploaded to SPIFFS
3. **I2C Errors** - Check SDA (GPIO6) and SCL (GPIO7) connections

### Debug Commands
```bash
# Check device list
pio device list

# Monitor serial output
pio device monitor

# Check SPIFFS contents
pio run --target uploadfs
```

## 📱 Web Interface Features

- **Responsive Design** - Works on mobile and desktop
- **Real-time Updates** - Live uptime, RSSI, and log monitoring
- **Interactive Controls** - Visual feedback for all operations
- **Error Handling** - Toast notifications for success/error states
- **Progress Tracking** - Visual progress bars for firmware updates

## 🎨 Customization

### Styling
- Edit `data/styles.css` for visual changes
- Modify colour schemes, layouts, and animations
- Add new CSS classes for additional components

### Functionality
- Extend `data/script.js` with new features
- Add new API endpoints in `main.cpp`
- Create new modular classes for additional functionality

### HTML Structure
- Modify `data/index.html` for layout changes
- Add new sections and controls
- Integrate with external CSS frameworks if desired

## 📚 Dependencies

- **FastLED** - RGB LED control
- **WiFiManager** - WiFi configuration
- **WebServer** - HTTP server functionality
- **SPIFFS** - File system for web interface
- **Wire** - I2C communication

## 🤝 Contributing

1. **Fork the repository**
2. **Create a feature branch**
3. **Make your changes**
4. **Test locally** using the HTML files
5. **Submit a pull request**

## 📄 License

This project is open source and available under the MIT License.

---

**Happy coding! 🚀**
