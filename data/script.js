// Global variables
console.log('Script.js loaded successfully!');
let autoRefreshInterval = null;
let modalAutoRefreshInterval = null; // Separate auto-refresh for modal
let selectedFile = null;
let firmwareData = null;
let firmwareDataTable = null; // DataTable instance

// Initialize when page loads
$(document).ready(function() {
    // Remove any time-related status bars that might exist
    $('.status').each(function() {
        if ($(this).text().includes('Current Time')) {
            $(this).remove();
        }
    });
    
    refreshLog();
    // Add a small delay to ensure everything is ready before loading firmware table
    setTimeout(() => {
        refreshFirmwareTable(); // Load firmware table on page load
    }, 100);
    startPeriodicUpdates();
    
    // Add responsive behavior
    setupResponsiveBehavior();
    
    // Enable auto-refresh by default
    setTimeout(() => {
        autoRefresh();
    }, 500);
    
    // Initialize auto-scroll status
    document.getElementById('autoScrollStatus').textContent = 'ON';
    document.getElementById('logEntries').classList.add('auto-scroll');
});

// Responsive behavior setup
function setupResponsiveBehavior() {
    // Handle window resize
    $(window).on('resize', function() {
        adjustLayoutForScreenSize();
    });
    
    // Initial layout adjustment
    adjustLayoutForScreenSize();
    
    // Add touch-friendly interactions for mobile
    if ($(window).width() < 768) {
        setupMobileInteractions();
    }
}

function adjustLayoutForScreenSize() {
    const width = $(window).width();
    
    if (width < 480) {
        // Extra small screens
        $('.container').addClass('xs-screen');
        $('.colour-btn').addClass('xs-size');
    } else if (width < 768) {
        // Small screens
        $('.container').addClass('sm-screen');
        $('.colour-btn').addClass('sm-size');
    } else if (width < 1024) {
        // Medium screens
        $('.container').addClass('md-screen');
        $('.colour-btn').addClass('md-size');
    } else {
        // Large screens
        $('.container').removeClass('xs-screen sm-screen md-screen');
        $('.colour-btn').removeClass('xs-size sm-size md-size');
    }
}

function setupMobileInteractions() {
    // Add touch-friendly button interactions
    $('.colour-btn, .control-btn, .i2c-btn').on('touchstart', function() {
        $(this).addClass('touch-active');
    }).on('touchend touchcancel', function() {
        $(this).removeClass('touch-active');
    });
    
    // Optimize table for mobile
    $('.firmware-table').addClass('mobile-table');
}

// LED Control Functions
function setLED(colour) {
    $.get('/led?colour=' + colour)
        .done(function(data) {
            console.log('LED set to:', colour);
            showNotification('LED set to ' + colour, 'success');
        })
        .fail(function(error) {
            console.error('Error setting LED:', error);
            showNotification('Failed to set LED', 'error');
        });
}

// I2C Command Functions
function sendI2CCommand(command) {
    $.get('/i2ccmd?cmd=' + command)
        .done(function(data) {
            console.log('I2C command sent:', data);
            showNotification('I2C command sent: 0x' + command.toString(16), 'success');
            setTimeout(refreshLog, 500);
        })
        .fail(function(error) {
            console.error('Error sending I2C command:', error);
            showNotification('Failed to send I2C command', 'error');
        });
}

// Firmware Management Functions
// Note: These functions are no longer used but kept for potential future use

function refreshFirmwareTable() {
    console.log('Refreshing firmware table...');
    $.get('/firmware/all')
        .done(function(data) {
            console.log('Firmware data received:', data);
            console.log('Data length:', data.length);
            console.log('Data type:', typeof data);
            console.log('Data contains "No firmware packages found":', data.includes('No firmware packages found'));
            
            if (data.includes('No firmware packages found')) {
                console.log('No firmware found, this might be a timing issue');
            }
            
            populateFirmwareTable(data);
        })
        .fail(function(error) {
            console.error('Error refreshing firmware table:', error);
            console.error('Error details:', error.responseText);
            // Don't show notification for automatic refresh on page load
        });
}

function populateFirmwareTable(firmwareData) {
    console.log('Populating firmware table with data:', firmwareData);
    console.log('Data contains "No firmware packages found":', firmwareData.includes('No firmware packages found'));
    console.log('Data contains "---":', firmwareData.includes('---'));
    
    if (firmwareData.includes('No firmware packages found')) {
        console.log('No firmware packages found, showing empty table');
        if (firmwareDataTable) {
            firmwareDataTable.destroy();
        }
        $('#firmwareTableBody').html('<tr><td colspan="6">No firmware packages found</td></tr>');
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
    
    // Prepare data for DataTable
    const tableData = [];
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
        
        // Add to table data array with row index for expandable functionality
        tableData.push([
            filename,
            displaySize,
            version,
            board,
            buildDate,
            `<button class="action-btn info" onclick="toggleFirmwareInfo('${filename}', ${index})" title="Toggle Info"><i class="fas fa-info-circle"></i></button>
             <button class="action-btn delete" onclick="deleteFirmware('${filename}')" title="Delete"><i class="fas fa-trash"></i></button>`
        ]);
    });
    
    // Destroy existing DataTable if it exists
    if (firmwareDataTable) {
        firmwareDataTable.destroy();
    }
    
    // Initialize DataTable with custom sorting
    firmwareDataTable = $('#firmwareTable').DataTable({
        data: tableData,
        columns: [
            { title: 'Filename' },
            { title: 'Size' },
            { title: 'Version' },
            { title: 'Board' },
            { title: 'Build Date' },
            { title: 'Actions', orderable: false }
        ],
        order: [
            [3, 'asc'],  // Board (device type) first
            [2, 'desc']  // Version second (newest first)
        ],
        pageLength: 10,
        lengthMenu: [[5, 10, 25, 50], [5, 10, 25, 50]],
        responsive: true,
        language: {
            search: "Search firmware:",
            lengthMenu: "Show _MENU_ entries per page",
            info: "Showing _START_ to _END_ of _TOTAL_ firmware packages",
            paginate: {
                first: "First",
                previous: "Previous",
                next: "Next",
                last: "Last"
            }
        },
        dom: '<"top"lf>rt<"bottom"ip><"clear">',
        initComplete: function() {
            // Add custom styling after table is initialized
            this.api().columns.adjust();
        }
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
    // For DataTables, we'll show the info in a modal or expand the row
    // For now, let's load and display the metadata directly
    loadFirmwareMetadata(filename, index);
}

function loadFirmwareMetadata(filename, index) {
    // For DataTables, we'll show the info in a notification or alert
    // Fetch the actual firmware package info to get real metadata
    $.get(`/firmware/package/info?filename=${encodeURIComponent(filename)}`)
        .done(function(data) {
            // Parse the real metadata from the .bin file
            const features = parseRealFirmwareMetadata(data, filename);
            showFirmwareInfoModal(features, filename);
        })
        .fail(function(error) {
            console.error('Error loading firmware metadata:', error);
            showNotification('Failed to load firmware features', 'error');
        });
}

function showFirmwareInfoModal(features, filename) {
    // Format the features content properly
    const formattedFeatures = formatFirmwareFeatures(features);
    
    // Create a modal to display firmware information
    const modalHtml = `
        <div id="firmwareModal" class="firmware-modal">
            <div class="firmware-modal-content">
                <div class="firmware-modal-header">
                    <h3><i class="fas fa-info-circle"></i> Firmware Information: ${filename}</h3>
                    <span class="firmware-modal-close" onclick="closeFirmwareModal()">&times;</span>
                </div>
                <div class="firmware-modal-body">
                    ${formattedFeatures}
                </div>
            </div>
        </div>
    `;
    
    // Remove existing modal if any
    $('.firmware-modal').remove();
    
    // Add modal to body
    $('body').append(modalHtml);
    
    // Show modal
    $('#firmwareModal').fadeIn(300);
}

function formatFirmwareFeatures(features) {
    // Split the features string and format it properly
    const lines = features.split(',');
    let formattedHtml = '';
    
    lines.forEach((line, index) => {
        const trimmedLine = line.trim();
        if (trimmedLine === '') return;
        
        if (index === 0) {
            // First line is the description
            formattedHtml += `<div class="firmware-info-description">${trimmedLine}</div>`;
        } else if (index === 1) {
            // Second line is the "Firmware Features:" title
            formattedHtml += `<div class="firmware-info-title">${trimmedLine}</div>`;
        } else {
            // Remaining lines are features - remove any existing bullet points and add clean ones
            const cleanFeature = trimmedLine.replace(/^•\s*/, ''); // Remove existing bullet points
            formattedHtml += `<div class="firmware-info-feature">${cleanFeature}</div>`;
        }
    });
    
    return formattedHtml;
}

function closeFirmwareModal() {
    $('#firmwareModal').fadeOut(300, function() {
        $(this).remove();
    });
}

function parseRealFirmwareMetadata(infoText, filename) {
    const features = [];
    const lines = infoText.split('\n');
    
    // Extract key information from the actual firmware package
    let description = 'Unknown';
    let featuresList = 'Unknown';
    
    lines.forEach(line => {
        if (line.startsWith('Description:')) {
            description = line.split('Description:')[1].trim();
        } else if (line.startsWith('Features:')) {
            featuresList = line.split('Features:')[1].trim();
        }
    });
    
    // Add description with proper format
    if (description !== 'Unknown') {
        features.push(description);
    }
    
    // Add firmware features section
    features.push('Firmware Features:');
    
    if (featuresList !== 'Unknown') {
        // Parse the comma-separated features from the backend
        const featureArray = featuresList.split(',');
        featureArray.forEach(feature => {
            features.push(feature.trim()); // Don't add bullet points here
        });
    } else {
        // Fallback to default features if none provided
        features.push('WS2812B LED control');
        features.push('Button input with mode cycling');
        features.push('I2C slave communication');
        features.push('EEPROM address persistence');
        features.push('Firmware update framework');
    }
    
    // Return as a comma-separated string for the modal formatter
    return features.join(',');
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
            
            // Auto-refresh the firmware table to show the new upload with retry mechanism
            console.log('Upload successful, calling refreshFirmwareTable() with delay...');
            setTimeout(() => {
                console.log('First refresh attempt...');
                refreshFirmwareTable();
                
                // Try again after 2 seconds in case file system needs time
                setTimeout(() => {
                    console.log('Second refresh attempt...');
                    refreshFirmwareTable();
                }, 2000);
                
                // Try one more time after 5 seconds
                setTimeout(() => {
                    console.log('Third refresh attempt...');
                    refreshFirmwareTable();
                }, 5000);
            }, 1000);
            
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
            // Format the log data to look like a Linux tail -f log
            const formattedLog = formatLogDataLinux(data);
            const logContainer = document.getElementById('logEntries');
            const wasScrolledToBottom = isScrolledToBottom(logContainer);
            
            logContainer.innerHTML = formattedLog;
            
            // Auto-scroll to bottom if auto-scroll is enabled and user was at bottom
            if (autoScrollEnabled && wasScrolledToBottom) {
                scrollToBottom(logContainer);
            }
        })
        .catch(error => {
            console.error('Error refreshing log:', error);
            document.getElementById('logEntries').innerHTML = '<div class="log-line">Error loading log</div>';
        });
}

function isScrolledToBottom(element) {
    return Math.abs(element.scrollHeight - element.clientHeight - element.scrollTop) < 10;
}

function scrollToBottom(element) {
    element.scrollTop = element.scrollHeight;
}

function formatLogData(logData) {
    if (!logData || logData.trim() === '') {
        return '<div class="log-entry">No log entries found</div>';
    }
    
    // Split the log data into lines and process each line
    const lines = logData.split('\n');
    let formattedHtml = '';
    
    lines.forEach((line, index) => {
        const trimmedLine = line.trim();
        if (trimmedLine === '') return;
        
        // Check if line starts with a timestamp pattern [Xs]
        const timestampMatch = trimmedLine.match(/^(\[\d+[smh]\s*\d*[smh]?\s*\d*[smh]?\])/);
        
        if (timestampMatch) {
            // Line has timestamp - format it properly
            const timestamp = timestampMatch[1];
            const message = trimmedLine.substring(timestamp.length).trim();
            
            formattedHtml += '<div class="log-entry">';
            formattedHtml += '<span class="log-timestamp">' + timestamp + '</span>';
            if (message) {
                formattedHtml += '<span class="log-message"> ' + message + '</span>';
            }
            formattedHtml += '</div>';
        } else {
            // Regular line without timestamp
            formattedHtml += '<div class="log-entry">' + trimmedLine + '</div>';
        }
    });
    
    return formattedHtml;
}

function formatLogDataLinux(logData) {
    if (!logData || logData.trim() === '') {
        return '<div class="log-line">No log entries found</div>';
    }
    
    // Try to split by timestamp patterns first
    const timestampPattern = /(\[\d+[smh]\s*\d*[smh]?\s*\d*[smh]?\])/g;
    const parts = logData.split(timestampPattern);
    
    let formattedHtml = '';
    let currentEntry = '';
    
    for (let i = 0; i < parts.length; i++) {
        const part = parts[i].trim();
        if (part === '') continue;
        
        if (part.match(/^\[\d+[smh]\s*\d*[smh]?\s*\d*[smh]?\]$/)) {
            // This is a timestamp, start a new log entry
            if (currentEntry !== '') {
                // Close the previous entry if it exists
                formattedHtml += currentEntry + '</div>';
            }
            formattedHtml += '<div class="log-line"><span class="log-time">' + part + '</span> ';
            currentEntry = '';
        } else if (part.length > 0) {
            // This is log message content
            currentEntry += part;
        }
    }
    
    // Close the last entry if it exists
    if (currentEntry !== '') {
        formattedHtml += currentEntry + '</div>';
    }
    
    // If no proper formatting was applied, fall back to simple line splitting
    if (formattedHtml === '') {
        const lines = logData.split('\n');
        lines.forEach(line => {
            if (line.trim() !== '') {
                formattedHtml += '<div class="log-line">' + line + '</div>';
            }
        });
    }
    
    return formattedHtml;
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

// Global variables for auto-scroll control
let autoScrollEnabled = true;

function autoRefresh() {
    const autoRefreshStatus = document.getElementById('autoRefreshStatus');
    const logEntries = document.getElementById('logEntries');
    
    if (autoRefreshInterval) {
        clearInterval(autoRefreshInterval);
        autoRefreshInterval = null;
        autoRefreshStatus.textContent = 'OFF';
        logEntries.classList.remove('following');
        showNotification('Auto-refresh disabled', 'info');
    } else {
        autoRefreshInterval = setInterval(refreshLog, 2000);
        autoRefreshStatus.textContent = 'ON (2s)';
        logEntries.classList.add('following');
        showNotification('Auto-refresh enabled', 'success');
    }
}

function toggleAutoScroll() {
    autoScrollEnabled = !autoScrollEnabled;
    const logEntries = document.getElementById('logEntries');
    const autoScrollStatus = document.getElementById('autoScrollStatus');
    
    if (autoScrollEnabled) {
        logEntries.classList.add('auto-scroll');
        autoScrollStatus.textContent = 'ON';
        scrollToBottom(logEntries);
        showNotification('Auto-scroll enabled', 'success');
    } else {
        logEntries.classList.remove('auto-scroll');
        autoScrollStatus.textContent = 'OFF';
        showNotification('Auto-scroll disabled', 'info');
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



// Modal Functions
function showDeviceInfoModal() {
    const modal = document.getElementById('deviceInfoModal');
    modal.style.display = 'block';
    
    // Refresh device info data
    fetch('/uptime')
        .then(response => response.json())
        .then(data => {
            if (document.getElementById('modalUptime')) {
                document.getElementById('modalUptime').textContent = data.uptime;
            }
            if (document.getElementById('modalMacAddress')) {
                document.getElementById('modalMacAddress').textContent = data.mac;
            }
            if (document.getElementById('modalIpAddress')) {
                document.getElementById('modalIpAddress').textContent = data.ip;
            }
            if (document.getElementById('modalRssi')) {
                document.getElementById('modalRssi').textContent = data.rssi + ' dBm';
            }
        })
        .catch(error => console.error('Error updating device info:', error));
}

function closeDeviceInfoModal() {
    const modal = document.getElementById('deviceInfoModal');
    modal.style.display = 'none';
}

function showSystemLogModal() {
    const modal = document.getElementById('systemLogModal');
    modal.style.display = 'block';
    
    // Initialize modal status elements with default values
    const autoRefreshStatus = document.getElementById('modalAutoRefreshStatus');
    const autoScrollStatus = document.getElementById('modalAutoScrollStatus');
    const logEntries = document.getElementById('modalLogEntries');
    
    if (autoRefreshStatus) {
        // Always start with OFF status for modal auto-refresh
        autoRefreshStatus.textContent = 'OFF';
    }
    
    if (autoScrollStatus && logEntries) {
        // Enable auto-scroll by default and update status
        logEntries.classList.add('auto-scroll');
        autoScrollStatus.textContent = 'ON';
    }
    
    // Refresh log data specifically for the modal
    refreshLogForModal();
    
    // Auto-scroll to bottom after a short delay to ensure content is loaded
    setTimeout(() => {
        if (logEntries && logEntries.classList.contains('auto-scroll')) {
            logEntries.scrollTop = logEntries.scrollHeight;
        }
    }, 100);
}

// Function to refresh log specifically for the modal
function refreshLogForModal() {
    fetch('/log')
        .then(response => response.text())
        .then(data => {
            const logEntries = document.getElementById('modalLogEntries');
            if (logEntries) {
                logEntries.innerHTML = formatLogData(data);
                
                // Apply auto-scroll if enabled
                if (logEntries.classList.contains('auto-scroll')) {
                    logEntries.scrollTop = logEntries.scrollHeight;
                }
            }
        })
        .catch(error => {
            console.error('Error fetching log:', error);
            const logEntries = document.getElementById('modalLogEntries');
            if (logEntries) {
                logEntries.innerHTML = 'Error loading log: ' + error.message;
            }
        });
}

// Function to clear log specifically for the modal
function clearLogForModal() {
    fetch('/log/clear', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
            console.log('Log cleared:', data);
            refreshLogForModal();
            showNotification('Log cleared successfully', 'success');
        })
        .catch(error => {
            console.error('Error clearing log:', error);
            showNotification('Failed to clear log', 'error');
        });
}

// Function to toggle auto-refresh specifically for the modal
function autoRefreshForModal() {
    const statusElement = document.getElementById('modalAutoRefreshStatus');
    if (modalAutoRefreshInterval) {
        clearInterval(modalAutoRefreshInterval);
        modalAutoRefreshInterval = null;
        if (statusElement) {
            statusElement.textContent = 'OFF';
        }
        showNotification('Auto-refresh disabled', 'info');
    } else {
        modalAutoRefreshInterval = setInterval(refreshLogForModal, 2000);
        if (statusElement) {
            statusElement.textContent = 'ON (2s)';
        }
        showNotification('Auto-refresh enabled (2s)', 'success');
    }
}

// Function to toggle auto-scroll specifically for the modal
function toggleAutoScrollForModal() {
    const logEntries = document.getElementById('modalLogEntries');
    const statusElement = document.getElementById('modalAutoScrollStatus');
    
    if (logEntries && statusElement) {
        if (logEntries.classList.contains('auto-scroll')) {
            logEntries.classList.remove('auto-scroll');
            statusElement.textContent = 'OFF';
            showNotification('Auto-scroll disabled', 'info');
        } else {
            logEntries.classList.add('auto-scroll');
            statusElement.textContent = 'ON';
            showNotification('Auto-scroll enabled', 'success');
        }
    }
}

function closeSystemLogModal() {
    const modal = document.getElementById('systemLogModal');
    modal.style.display = 'none';
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
