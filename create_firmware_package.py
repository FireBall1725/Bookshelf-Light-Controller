#!/usr/bin/env python3
"""
ESP32 Firmware Package Creator
Creates custom .bin packages with the format: [Magic][MetadataLength][Metadata][Firmware]
"""

import json
import struct
import sys
import os

MAGIC_HEADER = b"FLFW\0"  # 5 bytes

def create_firmware_package(metadata_file, firmware_file, output_file):
    """Create a firmware package from metadata and firmware files."""
    
    # Read metadata
    with open(metadata_file, 'r') as f:
        metadata_content = f.read()
    
    # Read firmware
    with open(firmware_file, 'rb') as f:
        firmware_content = f.read()
    
    # Create package
    with open(output_file, 'wb') as f:
        # Write magic header
        f.write(MAGIC_HEADER)
        
        # Write metadata length (4 bytes, little-endian)
        metadata_length = len(metadata_content)
        f.write(struct.pack('<I', metadata_length))
        
        # Write metadata
        f.write(metadata_content.encode('utf-8'))
        
        # Write firmware
        f.write(firmware_content)
    
    print(f"Created firmware package: {output_file}")
    print(f"  Magic header: {MAGIC_HEADER}")
    print(f"  Metadata length: {metadata_length} bytes")
    print(f"  Firmware size: {len(firmware_content)} bytes")
    print(f"  Total package size: {9 + metadata_length + len(firmware_content)} bytes")

def create_sample_metadata():
    """Create a sample metadata file for testing."""
    sample_metadata = {
        "firmware": {
            "version": "1.0.1",
            "description": "I2C Light Controller for ATtiny1616 with WS2812B LED"
        },
        "target": {
            "board": "ATtiny1616",
            "platform": "atmelmegaavr",
            "framework": "arduino",
            "upload_protocol": "serialupdi"
        },
        "features": [
            "WS2812B LED control",
            "Button input with mode cycling",
            "I2C slave communication",
            "EEPROM address persistence",
            "Firmware update framework"
        ],
        "build_info": {
            "timestamp": "2025-01-30T12:00:00.000000",
            "platformio_version": "6.1.0",
            "python_version": "3.11.0"
        },
        "files": {
            "firmware_hex": {
                "name": "firmware.hex",
                "size_bytes": 16471,
                "size_kb": 16.08,
                "description": "Intel HEX firmware file for ATtiny1616"
            }
        }
    }
    
    with open('sample_metadata.json', 'w') as f:
        json.dump(sample_metadata, f, indent=2)
    
    print("Created sample metadata file: sample_metadata.json")

def main():
    if len(sys.argv) == 1:
        print("ESP32 Firmware Package Creator")
        print("Usage:")
        print("  python3 create_firmware_package.py create <metadata.json> <firmware.hex> <output.bin>")
        print("  python3 create_firmware_package.py sample")
        print()
        print("Examples:")
        print("  python3 create_firmware_package.py sample")
        print("  python3 create_firmware_package.py create sample_metadata.json firmware.hex firmware-v1.0.1.bin")
        return
    
    command = sys.argv[1]
    
    if command == "sample":
        create_sample_metadata()
    elif command == "create":
        if len(sys.argv) != 5:
            print("Error: create command requires 3 arguments")
            print("Usage: python3 create_firmware_package.py create <metadata.json> <firmware.hex> <output.bin>")
            return
        
        metadata_file = sys.argv[2]
        firmware_file = sys.argv[3]
        output_file = sys.argv[4]
        
        if not os.path.exists(metadata_file):
            print(f"Error: Metadata file '{metadata_file}' not found")
            return
        
        if not os.path.exists(firmware_file):
            print(f"Error: Firmware file '{firmware_file}' not found")
            return
        
        create_firmware_package(metadata_file, firmware_file, output_file)
    else:
        print(f"Unknown command: {command}")

if __name__ == "__main__":
    main()
