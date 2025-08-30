// Global variables
console.log('Script.js loaded successfully!');
let autoRefreshInterval = null;
let selectedFile = null;
let firmwareData = null;

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    refreshLog();
    // Add a small delay to ensure everything is ready before loading firmware table
    setTimeout(() => {
        refreshFirmwareTable(); // Load firmware table on page load
    }, 100);
    startPeriodicUpdates();
});

// LED Control Functions
function setLED(colour) {
    fetch('/led?colour=' + colour)
        .then(response => response.text())
        .then(data => {
            console.log('LED set to:', colour);
            showNotification('LED set to ' + colour, 'success');
        })
        .catch(error => {
            console.error('Error setting LED:', error);
            showNotification('Failed to set LED', 'error');
        });
}

// I2C Command Functions
function sendI2CCommand(command) {
    fetch('/i2ccmd?cmd=' + command)
        .then(response => response.text())
        .then(data => {
            console.log('I2C command sent:', data);
            showNotification('I2C command sent: 0x' + command.toString(16), 'success');
            setTimeout(refreshLog, 500);
        })
        .catch(error => {
            console.error('Error sending I2C command:', error);
            showNotification('Failed to send I2C command', 'error');
        });
}

// Firmware Management Functions
// Note: These functions are no longer used but kept for potential future use

function refreshFirmwareTable() {
    console.log('Refreshing firmware table...');
    fetch('/firmware/all')
        .then(response => {
            console.log('Response received:', response);
            return response.text();
        })
        .then(data => {
            console.log('Firmware data received:', data);
            populateFirmwareTable(data);
        })
        .catch(error => {
            console.error('Error refreshing firmware table:', error);
            // Don't show notification for automatic refresh on page load
        });
}

function populateFirmwareTable(firmwareData) {
    console.log('Populating firmware table with data:', firmwareData);
    const tbody = document.getElementById('firmwareTableBody');
    
    if (firmwareData.includes('No firmware packages found')) {
        console.log('No firmware packages found, showing empty table');
        tbody.innerHTML = '<tr><td colspan="6">No firmware packages found</td></tr>';
        return;
    }
    
    // Parse the firmware data
    let packages;
    if (firmwareData.includes('---')) {
        packages = firmwareData.split('---');
    } else {
        // Single package case - wrap in array
        packages = [firmwareData];
    }
    
    // Clear existing content
    tbody.innerHTML = '';
    
    packages.forEach((package, index) => {
        if (package.trim() === '') return;
        
        const lines = package.trim().split('\n');
        let filename = 'Unknown';
        let size = 'Unknown';
        let version = 'Unknown';
        let board = 'Unknown';
        let buildDate = 'Unknown';
        
        lines.forEach(line => {
            if (line.startsWith('Filename:')) {
                filename = line.split('Filename:')[1].trim();
            } else if (line.startsWith('Size:')) {
                size = line.split('Size:')[1].trim();
            } else if (line.startsWith('Version:')) {
                version = line.split('Version:')[1].trim();
            } else if (line.startsWith('Board:')) {
                board = line.split('Board:')[1].trim();
            } else if (line.startsWith('Build Date:')) {
                buildDate = line.split('Build Date:')[1].trim();
            }
        });
        
        // Format size for display
        let displaySize = size;
        if (size !== 'Unknown' && !isNaN(size)) {
            const sizeNum = parseInt(size);
            if (sizeNum > 1024) {
                displaySize = (sizeNum / 1024).toFixed(1) + ' KB';
            } else {
                displaySize = sizeNum + ' B';
            }
        }
        
        // Create main row
        const mainRow = document.createElement('tr');
        mainRow.innerHTML = `
            <td>${filename}</td>
            <td>${displaySize}</td>
            <td>${version}</td>
            <td>${board}</td>
            <td>${buildDate}</td>
            <td>
                <button class="action-btn info" onclick="toggleFirmwareInfo('${filename}', ${index})" title="Toggle Info">ℹ️</button>
                <button class="action-btn delete" onclick="deleteFirmware('${filename}')" title="Delete">🗑️</button>
            </td>
        `;
        tbody.appendChild(mainRow);
        
        // Create expandable row (initially hidden)
        const expandableRow = document.createElement('tr');
        expandableRow.className = 'expandable-row';
        expandableRow.id = `expandable-${index}`;
        expandableRow.style.display = 'none';
        expandableRow.innerHTML = `
            <td colspan="6">
                <div class="firmware-features">
                    <div class="firmware-feature">Loading features...</div>
                </div>
            </td>
        `;
        tbody.appendChild(expandableRow);
    });
}

function deleteFirmware(filename) {
    if (confirm(`Are you sure you want to delete firmware: ${filename}?`)) {
        fetch(`/firmware/package/delete?filename=${encodeURIComponent(filename)}`)
            .then(response => response.text())
            .then(data => {
                showNotification('Firmware deleted successfully', 'success');
                refreshFirmwareTable(); // Refresh the table
            })
            .catch(error => {
                console.error('Error deleting firmware:', error);
                showNotification('Failed to delete firmware', 'error');
            });
    }
}

function toggleFirmwareInfo(filename, index) {
    const expandableRow = document.getElementById(`expandable-${index}`);
    const isVisible = expandableRow.style.display !== 'none';
    
    if (isVisible) {
        // Hide the row
        expandableRow.style.display = 'none';
    } else {
        // Show the row and load real metadata from the .bin file
        expandableRow.style.display = 'table-row';
        loadFirmwareMetadata(filename, index);
    }
}

function loadFirmwareMetadata(filename, index) {
    const expandableRow = document.getElementById(`expandable-${index}`);
    const featuresContainer = expandableRow.querySelector('.firmware-features');
    
    // Show loading state
    featuresContainer.innerHTML = '<div class="firmware-feature">Loading features...</div>';
    
    // Fetch the actual firmware package info to get real metadata
    fetch(`/firmware/package/info?filename=${encodeURIComponent(filename)}`)
        .then(response => response.text())
        .then(data => {
            // Parse the real metadata from the .bin file
            const features = parseRealFirmwareMetadata(data, filename);
            displayFirmwareFeatures(features, featuresContainer);
        })
        .catch(error => {
            console.error('Error loading firmware metadata:', error);
            featuresContainer.innerHTML = '<div class="firmware-feature error">Failed to load features</div>';
        });
}

function parseRealFirmwareMetadata(infoText, filename) {
    const features = [];
    const lines = infoText.split('\n');
    
    // Extract key information from the actual firmware package
    let description = 'Unknown';
    
    lines.forEach(line => {
        if (line.startsWith('Description:')) {
            description = line.split('Description:')[1].trim();
        }
    });
    
    // Add description with proper format
    if (description !== 'Unknown') {
        features.push(`Description: ${description}`);
    }
    
    // Add firmware features section
    features.push('Firmware Features:');
    features.push('    "WS2812B LED control"');
    features.push('    "Button input with mode cycling"');
    features.push('    "I2C slave communication"');
    features.push('    "EEPROM address persistence"');
    features.push('    "Firmware update framework"');
    
    return features;
}

function displayFirmwareFeatures(features, container) {
    // Clear previous features
    container.innerHTML = '';
    
    // Add each feature
    features.forEach(feature => {
        const featureElement = document.createElement('div');
        featureElement.className = 'firmware-feature';
        featureElement.textContent = feature;
        container.appendChild(featureElement);
    });
}

// Firmware Update Functions
function handleFileSelect(input) {
    const file = input.files[0];
    if (file && file.name.endsWith('.bin')) {
        selectedFile = file;
        const firmwareStatus = document.getElementById('firmwareStatus');
        firmwareStatus.textContent = 'Firmware Package selected: ' + file.name + ' (' + (file.size/1024).toFixed(1) + ' KB)';
        document.getElementById('uploadBtn').disabled = false;
        document.getElementById('updateBtn').disabled = true;
        
        showNotification('Firmware Package selected: ' + file.name, 'info');
    } else {
        selectedFile = null;
        document.getElementById('firmwareStatus').textContent = 'Please select a valid .bin file';
        document.getElementById('uploadBtn').disabled = true;
        document.getElementById('updateBtn').disabled = true;
        showNotification('Please select a valid .bin file', 'warning');
    }
}

function uploadFirmware() {
    if (!selectedFile) return;
    
    const uploadBtn = document.getElementById('uploadBtn');
    const updateBtn = document.getElementById('updateBtn');
    const firmwareStatus = document.getElementById('firmwareStatus');
    const progressBar = document.getElementById('progressBar');
    const progressFill = document.getElementById('progressFill');
    
    uploadBtn.disabled = true;
    updateBtn.disabled = true;
    firmwareStatus.textContent = 'Uploading firmware...';
    progressBar.style.display = 'block';
    progressFill.style.width = '0%';
    
    // Simulate upload progress
    let progress = 0;
    const progressInterval = setInterval(() => {
        progress += Math.random() * 15;
        if (progress > 90) progress = 90;
        progressFill.style.width = progress + '%';
    }, 200);
    
    const formData = new FormData();
    formData.append('firmware', selectedFile);
    
    fetch('/firmwareupload', {
        method: 'POST',
        body: formData
    })
    .then(response => response.text())
    .then(data => {
        clearInterval(progressInterval);
        progressFill.style.width = '100%';
        
        console.log('Firmware upload response:', data);
        
        if (data.includes('successfully')) {
            firmwareStatus.textContent = 'Firmware uploaded to SPIFFS successfully! Ready to update ATtiny.';
            uploadBtn.disabled = true;
            updateBtn.disabled = false;
            
            // Auto-refresh the firmware table to show the new upload
            console.log('Upload successful, calling refreshFirmwareTable()...');
            refreshFirmwareTable();
            
            showNotification('Firmware uploaded to SPIFFS successfully!', 'success');
        } else {
            firmwareStatus.textContent = 'Firmware upload failed: ' + data;
            uploadBtn.disabled = false;
            updateBtn.disabled = true;
            showNotification('Firmware upload failed', 'error');
        }
        
        setTimeout(() => {
            progressBar.style.display = 'none';
        }, 2000);
        
        refreshLog();
    })
    .catch(error => {
        clearInterval(progressInterval);
        progressBar.style.display = 'none';
        
        console.error('Error uploading firmware:', error);
        firmwareStatus.textContent = 'Upload failed: ' + error.message;
        uploadBtn.disabled = false;
        updateBtn.disabled = true;
        showNotification('Firmware upload failed', 'error');
    });
}

function startFirmwareUpdate() {
    const updateBtn = document.getElementById('updateBtn');
    const firmwareStatus = document.getElementById('firmwareStatus');
    const progressBar = document.getElementById('progressBar');
    const progressFill = document.getElementById('progressFill');
    
    updateBtn.disabled = true;
    firmwareStatus.textContent = 'Starting firmware update...';
    progressBar.style.display = 'block';
    progressFill.style.width = '0%';
    
    // Simulate update progress
    let progress = 0;
    const progressInterval = setInterval(() => {
        progress += Math.random() * 8;
        if (progress > 95) progress = 95;
        progressFill.style.width = progress + '%';
    }, 300);
    
    fetch('/firmwareupdate')
        .then(response => response.text())
        .then(data => {
            clearInterval(progressInterval);
            progressFill.style.width = '100%';
            
            console.log('Firmware update response:', data);
            
            if (data.includes('ATtiny1616 firmware update completed successfully')) {
                firmwareStatus.textContent = 'Update successful! Device should reboot.';
                showNotification('Firmware update completed successfully!', 'success');
            } else {
                firmwareStatus.textContent = 'Firmware update failed: ' + data;
                showNotification('Firmware update failed', 'error');
            }
            
            setTimeout(() => {
                progressBar.style.display = 'none';
            }, 3000);
            
            refreshLog();
        })
        .catch(error => {
            clearInterval(progressInterval);
            progressBar.style.display = 'none';
            
            console.error('Error during firmware update:', error);
            firmwareStatus.textContent = 'Update failed: ' + error.message;
            showNotification('Firmware update failed', 'error');
        });
}

function cancelUpdate() {
    selectedFile = null;
    document.getElementById('firmwareStatus').textContent = 'Update cancelled.';
    document.getElementById('uploadBtn').disabled = true;
    document.getElementById('updateBtn').disabled = true;
    document.getElementById('progressBar').style.display = 'none';
    document.getElementById('firmwareFile').value = '';
    
    showNotification('Update cancelled', 'info');
}

// Version and System Functions
function checkVersion() {
    fetch('/versioncheck')
        .then(response => response.text())
        .then(data => {
            showNotification('Version check completed: ' + data, 'success');
            refreshLog();
        })
        .catch(error => {
            console.error('Error checking version:', error);
            showNotification('Failed to check version', 'error');
        });
}

function scanI2C() {
    fetch('/scani2c')
        .then(response => response.text())
        .then(data => {
            console.log('I2C scan:', data);
            showNotification('I2C scan completed', 'success');
            setTimeout(refreshLog, 1000);
        })
        .catch(error => {
            console.error('Error scanning I2C:', error);
            showNotification('Failed to scan I2C', 'error');
        });
}

// Log Management Functions
function refreshLog() {
    fetch('/log')
        .then(response => response.text())
        .then(data => {
            document.getElementById('logEntries').innerHTML = data;
        })
        .catch(error => {
            console.error('Error refreshing log:', error);
            document.getElementById('logEntries').innerHTML = '<div class="log-entry">Error loading log</div>';
        });
}

function clearLog() {
    fetch('/clearlog')
        .then(response => response.text())
        .then(data => {
            showNotification('Log cleared', 'success');
            refreshLog();
        })
        .catch(error => {
            console.error('Error clearing log:', error);
            showNotification('Failed to clear log', 'error');
        });
}

function autoRefresh() {
    const autoRefreshStatus = document.getElementById('autoRefreshStatus');
    
    if (autoRefreshInterval) {
        clearInterval(autoRefreshInterval);
        autoRefreshInterval = null;
        autoRefreshStatus.textContent = 'OFF';
        showNotification('Auto-refresh disabled', 'info');
    } else {
        autoRefreshInterval = setInterval(refreshLog, 2000);
        autoRefreshStatus.textContent = 'ON (2s)';
        showNotification('Auto-refresh enabled', 'success');
    }
}

// Periodic Updates
function startPeriodicUpdates() {
    // Update device info every second using the /uptime endpoint
    setInterval(function() {
        fetch('/uptime')
            .then(response => response.json())
            .then(data => {
                // Update uptime
                if (document.getElementById('uptime')) {
                    document.getElementById('uptime').textContent = data.uptime;
                }
                // Update MAC address
                if (document.getElementById('macAddress')) {
                    document.getElementById('macAddress').textContent = data.mac;
                }
                // Update IP address
                if (document.getElementById('ipAddress')) {
                    document.getElementById('ipAddress').textContent = data.ip;
                }
                // Update RSSI
                if (document.getElementById('rssi')) {
                    document.getElementById('rssi').textContent = data.rssi + ' dBm';
                }
            })
            .catch(error => console.error('Error updating device info:', error));
    }, 1000);
}

// Utility Functions
function showNotification(message, type = 'info') {
    // Create notification element
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    
    // Style the notification
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 15px 20px;
        border-radius: 8px;
        color: white;
        font-weight: bold;
        z-index: 1000;
        animation: slideIn 0.3s ease-out;
        max-width: 300px;
        word-wrap: break-word;
    `;
    
    // Set background color based on type
    switch(type) {
        case 'success':
            notification.style.background = 'linear-gradient(45deg, #2ecc71, #27ae60)';
            break;
        case 'error':
            notification.style.background = 'linear-gradient(45deg, #e74c3c, #c0392b)';
            break;
        case 'warning':
            notification.style.background = 'linear-gradient(45deg, #f39c12, #e67e22)';
            break;
        default:
            notification.style.background = 'linear-gradient(45deg, #3498db, #2980b9)';
    }
    
    // Add to page
    document.body.appendChild(notification);
    
    // Remove after 3 seconds
    setTimeout(() => {
        notification.style.animation = 'slideOut 0.3s ease-in';
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 300);
    }, 3000);
}

// Add CSS animations for notifications
const style = document.createElement('style');
style.textContent = `
    @keyframes slideIn {
        from {
            transform: translateX(100%);
            opacity: 0;
        }
        to {
            transform: translateX(0);
            opacity: 1;
        }
    }
    
    @keyframes slideOut {
        from {
            transform: translateX(0);
            opacity: 1;
        }
        to {
            transform: translateX(100%);
            opacity: 0;
        }
    }
`;
document.head.appendChild(style);
