// Global variables
let autoRefreshInterval = null;
let selectedFile = null;
let firmwareData = null;

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    refreshLog();
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
function listStoredFirmware() {
    fetch('/firmware/list')
        .then(response => response.text())
        .then(data => {
            document.getElementById('storedFirmwareInfo').textContent = data;
            
            // If firmware is found, also get detailed info to populate version fields
            if (data.includes('attiny_firmware.hex')) {
                getFirmwareInfo();
            }
            
            showNotification('Stored firmware list retrieved', 'success');
        })
        .catch(error => {
            console.error('Error listing firmware:', error);
            showNotification('Failed to list stored firmware', 'error');
        });
}

function getFirmwareInfo() {
    // First check if we have any firmware packages
    fetch('/firmware/packages')
        .then(response => response.text())
        .then(data => {
            if (data.includes('No firmware packages found')) {
                // No packages, check legacy hex file
                fetch('/firmware/info')
                    .then(response => response.text())
                    .then(hexData => {
                        document.getElementById('storedFirmwareInfo').textContent = hexData;
                        showNotification('Legacy firmware info retrieved', 'info');
                    })
                    .catch(error => {
                        console.error('Error getting legacy firmware info:', error);
                        document.getElementById('storedFirmwareInfo').textContent = 'No firmware found';
                    });
                return;
            }
            
            // We have packages, get the first one's info
            const lines = data.split('\n');
            for (const line of lines) {
                if (line.includes('.bin') && line.includes('bytes')) {
                    const filename = line.match(/- (.+?) \(/)?.[1];
                    if (filename) {
                        // Get detailed info for this package
                        fetch(`/firmware/package/info?filename=${encodeURIComponent(filename)}`)
                            .then(response => response.text())
                            .then(packageData => {
                                document.getElementById('storedFirmwareInfo').textContent = packageData;
                                
                                // Parse package info and populate version fields
                                const packageLines = packageData.split('\n');
                                let version = 'Unknown';
                                let buildDate = 'Unknown';
                                let size = 'Unknown';
                                
                                for (const packageLine of packageLines) {
                                    if (packageLine.includes('Size:')) {
                                        size = packageLine.split('Size:')[1].trim();
                                    } else if (packageLine.includes('Version:')) {
                                        version = packageLine.split('Version:')[1].trim();
                                    } else if (packageLine.includes('Build Date:')) {
                                        buildDate = packageLine.split('Build Date:')[1].trim();
                                    }
                                }
                                
                                // Update version info display
                                document.getElementById('newVersion').textContent = 'New: ' + version;
                                document.getElementById('buildDate').textContent = 'Build: ' + buildDate;
                                
                                showNotification('Firmware package info retrieved', 'success');
                            })
                            .catch(error => {
                                console.error('Error getting package info:', error);
                                showNotification('Failed to get package info', 'error');
                            });
                        break;
                    }
                }
            }
        })
        .catch(error => {
            console.error('Error getting firmware packages:', error);
            showNotification('Failed to get firmware packages', 'error');
        });
}

function deleteStoredFirmware() {
    if (confirm('Are you sure you want to delete the stored firmware? This action cannot be undone.')) {
        // First check what type of firmware we have
        fetch('/firmware/packages')
            .then(response => response.text())
            .then(data => {
                if (data.includes('No firmware packages found')) {
                    // Delete legacy hex file
                    fetch('/firmware/delete?filename=attiny_firmware.hex')
                        .then(response => response.text())
                        .then(deleteData => {
                            document.getElementById('storedFirmwareInfo').textContent = deleteData;
                            showNotification('Legacy firmware deleted', 'success');
                            document.getElementById('updateBtn').disabled = true;
                        })
                        .catch(error => {
                            console.error('Error deleting legacy firmware:', error);
                            showNotification('Failed to delete legacy firmware', 'error');
                        });
                } else {
                    // Delete the first package found
                    const lines = data.split('\n');
                    for (const line of lines) {
                        if (line.includes('.bin') && line.includes('bytes')) {
                            const filename = line.match(/- (.+?) \(/)?.[1];
                            if (filename) {
                                fetch(`/firmware/package/delete?filename=${encodeURIComponent(filename)}`)
                                    .then(response => response.text())
                                    .then(deleteData => {
                                        document.getElementById('storedFirmwareInfo').textContent = deleteData;
                                        showNotification('Firmware package deleted', 'success');
                                        document.getElementById('updateBtn').disabled = true;
                                    })
                                    .catch(error => {
                                        console.error('Error deleting firmware package:', error);
                                        showNotification('Failed to delete firmware package', 'error');
                                    });
                                break;
                            }
                        }
                    }
                }
            })
            .catch(error => {
                console.error('Error checking firmware type:', error);
                showNotification('Failed to check firmware type', 'error');
            });
    }
}

// Firmware Update Functions
function handleFileSelect(input) {
    const file = input.files[0];
    if (file && (file.name.endsWith('.hex') || file.name.endsWith('.bin'))) {
        selectedFile = file;
        const firmwareStatus = document.getElementById('firmwareStatus');
        const isPackage = file.name.endsWith('.bin');
        const fileType = isPackage ? 'Firmware Package' : 'Firmware File';
        firmwareStatus.textContent = fileType + ' selected: ' + file.name + ' (' + (file.size/1024).toFixed(1) + ' KB)';
        document.getElementById('uploadBtn').disabled = false;
        document.getElementById('updateBtn').disabled = true;
        
        // Hide version info when new file is selected
        document.getElementById('versionInfo').style.display = 'none';
        
        showNotification(fileType + ' selected: ' + file.name, 'info');
    } else {
        selectedFile = null;
        document.getElementById('firmwareStatus').textContent = 'Please select a valid .hex or .bin file';
        document.getElementById('uploadBtn').disabled = true;
        document.getElementById('updateBtn').disabled = true;
        showNotification('Please select a valid .hex or .bin file', 'warning');
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
            
            // Show version info and populate with firmware details
            const versionInfo = document.getElementById('versionInfo');
            versionInfo.style.display = 'block';
            
            // Get firmware info to populate version details
            getFirmwareInfo();
            
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
    const versionInfo = document.getElementById('versionInfo');
    
    updateBtn.disabled = true;
    firmwareStatus.textContent = 'Starting firmware update...';
    progressBar.style.display = 'block';
    progressFill.style.width = '0%';
    versionInfo.style.display = 'none';
    
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
                versionInfo.style.display = 'block';
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
    document.getElementById('versionInfo').style.display = 'none';
    document.getElementById('firmwareFile').value = '';
    
    showNotification('Update cancelled', 'info');
}

// Version and System Functions
function checkVersion() {
    fetch('/versioncheck')
        .then(response => response.text())
        .then(data => {
            const versionInfo = document.getElementById('versionInfo');
            const currentVersion = document.getElementById('currentVersion');
            currentVersion.textContent = 'Current: ' + data;
            versionInfo.style.display = 'block';
            
            showNotification('Version check completed', 'success');
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
    // Update uptime every second
    setInterval(function() {
        fetch('/uptime')
            .then(response => response.text())
            .then(data => {
                document.getElementById('uptime').textContent = data;
            })
            .catch(error => console.error('Error updating uptime:', error));
    }, 1000);
    
    // Update RSSI every 5 seconds
    setInterval(function() {
        fetch('/rssi')
            .then(response => response.text())
            .then(data => {
                document.getElementById('rssi').textContent = data;
            })
            .catch(error => console.error('Error updating RSSI:', error));
    }, 5000);
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
