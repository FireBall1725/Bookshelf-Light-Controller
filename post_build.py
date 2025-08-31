#!/usr/bin/env python3
"""
Post-build script for PlatformIO
Generates firmware.meta JSON file and creates binary package with FLFW format
"""

import os
import json
import struct
import re
from datetime import datetime
from pathlib import Path

# Import PlatformIO environment
Import("env")

def post_build(source, target, env):
    """
    Post-build hook that runs after successful compilation
    """
    print("🔧 Running post-build script...")
    print(f"   Source: {source}")
    print(f"   Target: {target}")
    print(f"   Environment: {env['PIOENV']}")
    
    # Get build directory and firmware files
    build_dir = Path(env["PROJECT_DIR"]) / ".pio" / "build" / env["PIOENV"]
    
    # Check for ESP32 binary file first, then fall back to hex for ATtiny
    firmware_bin = build_dir / "firmware.bin"
    firmware_hex = build_dir / "firmware.hex"
    
    if firmware_bin.exists():
        print("✅ ESP32 firmware.bin found, proceeding with package creation...")
        firmware_file = firmware_bin
        firmware_type = "bin"
    elif firmware_hex.exists():
        print("✅ ATtiny firmware.hex found, proceeding with package creation...")
        firmware_file = firmware_hex
        firmware_type = "hex"
    else:
        print("⏳ No firmware file found yet, skipping...")
        return
    
    # Extract version, board model, and description from main.cpp and update firmware.meta
    version, board_model, board_description = extract_values_from_source(env["PROJECT_DIR"])
    if not version:
        print("❌ Could not extract values from source!")
        return
    
    print(f"📋 Extracted from source:")
    print(f"   Version: {version}")
    print(f"   Board Model: {board_model}")
    print(f"   Description: {board_description}")
    
    # Read and update metadata with values from source
    metadata = read_and_update_metadata(env["PROJECT_DIR"], version, board_model, board_description)
    if not metadata:
        print("❌ Could not read/update metadata!")
        return
    
    # Update metadata with build information
    metadata = update_metadata_with_build_info(metadata, build_dir, firmware_file, firmware_type)
    
    # Write updated metadata to build directory
    meta_file = build_dir / "firmware.meta"
    with open(meta_file, 'w') as f:
        json.dump(metadata, f, indent=2)
    print(f"✅ Generated {meta_file}")
    
    if firmware_type == "bin":
        # For ESP32, just copy the binary with versioned name
        bin_filename = f"firmware-v{version}.bin"
        bin_path = build_dir / bin_filename
        
        # Copy firmware with versioned name
        with open(firmware_file, 'rb') as src, open(bin_path, 'wb') as dst:
            dst.write(src.read())
        print(f"✅ Created {bin_path}")
        
        # Copy to project root for easy access
        project_bin = Path(env["PROJECT_DIR"]) / bin_filename
        with open(bin_path, 'rb') as src, open(project_bin, 'wb') as dst:
            dst.write(src.read())
        print(f"✅ Copied to {project_bin}")
    else:
        # For ATtiny, create binary package with FLFW format
        bin_filename = f"firmware-v{version}.bin"
        bin_path = build_dir / bin_filename
        
        create_binary_package(bin_path, metadata, firmware_file)
        print(f"✅ Created {bin_path}")
        
        # Copy to project root for easy access
        project_bin = Path(env["PROJECT_DIR"]) / bin_filename
        with open(bin_path, 'rb') as src, open(project_bin, 'wb') as dst:
            dst.write(src.read())
        print(f"✅ Copied to {project_bin}")
    
    print("🎉 Post-build script completed successfully!")

def extract_values_from_source(project_dir):
    """
    Extract version, board model, and description from main.cpp source file
    """
    main_cpp = Path(project_dir) / "src" / "main.cpp"
    
    if not main_cpp.exists():
        print(f"❌ {main_cpp} not found!")
        return None, None, None
    
    try:
        with open(main_cpp, 'r') as f:
            content = f.read()
        
        # Look for FIRMWARE_VERSION definition
        version_pattern = r'#define\s+FIRMWARE_VERSION\s+"([^"]+)"'
        version_match = re.search(version_pattern, content)
        
        # Look for BOARD_MODEL definition
        model_pattern = r'#define\s+BOARD_MODEL\s+"([^"]+)"'
        model_match = re.search(model_pattern, content)
        
        # Look for BOARD_DESCRIPTION definition
        desc_pattern = r'#define\s+BOARD_DESCRIPTION\s+"([^"]+)"'
        desc_match = re.search(desc_pattern, content)
        
        # Debug output
        print(f"🔍 Debug - Found patterns:")
        print(f"   Version match: {version_match}")
        if version_match:
            print(f"   Version text: '{version_match.group(0)}' -> '{version_match.group(1)}'")
        print(f"   Model match: {model_match}")
        if model_match:
            print(f"   Model text: '{model_match.group(0)}' -> '{model_match.group(1)}'")
        print(f"   Description match: {desc_match}")
        if desc_match:
            print(f"   Description text: '{desc_match.group(0)}' -> '{desc_match.group(1)}'")
        
        # Also show the content around where we expect to find these
        print(f"🔍 Debug - Content around expected locations:")
        lines = content.split('\n')
        for i, line in enumerate(lines):
            if 'BOARD_MODEL' in line or 'BOARD_DESCRIPTION' in line or 'FIRMWARE_VERSION' in line:
                print(f"   Line {i+1}: {line.strip()}")
        
        version = version_match.group(1) if version_match else None
        board_model = model_match.group(1) if model_match else None
        board_description = desc_match.group(1) if desc_match else None
        
        if not version:
            print("❌ FIRMWARE_VERSION not found in source")
            return None, None, None
            
        if not board_model:
            print("❌ BOARD_MODEL not found in source")
            return None, None, None
            
        if not board_description:
            print("❌ BOARD_DESCRIPTION not found in source")
            return None, None, None
        
        return version, board_model, board_description
            
    except Exception as e:
        print(f"❌ Error reading source: {e}")
        return None, None, None

def read_and_update_metadata(project_dir, version, board_model, board_description):
    """
    Read metadata from firmware.meta file and update with extracted values
    """
    meta_file = Path(project_dir) / "firmware.meta"
    
    if not meta_file.exists():
        print(f"❌ {meta_file} not found!")
        return None
    
    try:
        with open(meta_file, 'r') as f:
            metadata = json.load(f)
        
        # Update metadata with values from source
        metadata["firmware"]["version"] = version
        metadata["firmware"]["description"] = board_description
        metadata["firmware"]["board"] = board_model
        
        print(f"📝 Updated metadata with values from source")
        
        return metadata
    except Exception as e:
        print(f"❌ Error reading/updating metadata: {e}")
        return None

def update_metadata_with_build_info(metadata, build_dir, firmware_file, firmware_type):
    """
    Update metadata with build-specific information
    """
    # Get file sizes
    file_size = firmware_file.stat().st_size if firmware_file.exists() else 0
    
    # Get build timestamp
    build_time = datetime.now().isoformat()
    
    # Get git hash if available
    git_hash = None
    try:
        import subprocess
        result = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], 
                              capture_output=True, text=True, cwd=build_dir.parent.parent)
        if result.returncode == 0:
            git_hash = result.stdout.strip()
    except:
        pass
    
    # Update metadata with build info
    metadata["build_info"] = {
        "timestamp": build_time,
        "git_hash": git_hash,
        "platformio_version": os.environ.get("PLATFORMIO_VERSION", "6.1.0"),
        "python_version": os.environ.get("PYTHON_VERSION", "3.11.0")
    }
    
    if firmware_type == "bin":
        metadata["files"] = {
            "firmware_bin": {
                "name": "firmware.bin",
                "size_bytes": file_size,
                "size_kb": round(file_size / 1024, 2),
                "description": "ESP32 firmware binary file"
            }
        }
    else:
        metadata["files"] = {
            "firmware_hex": {
                "name": "firmware.hex",
                "size_bytes": file_size,
                "size_kb": round(file_size / 1024, 2),
                "description": "Intel HEX firmware file for ATtiny1616"
            }
        }
    
    return metadata

def create_binary_package(bin_path, metadata, firmware_hex):
    """
    Create binary package with FLFW format:
    [FLFW\0][MetadataLength][JSON Metadata][Intel HEX Firmware]
    """
    try:
        with open(bin_path, 'wb') as bin_file:
            # Magic Header: FLFW\0 (5 bytes)
            bin_file.write(b'FLFW\0')
            
            # Convert metadata to JSON string
            json_metadata = json.dumps(metadata, separators=(',', ':'))
            metadata_bytes = json_metadata.encode('utf-8')
            
            # Metadata Length: 4-byte little-endian integer
            metadata_length = len(metadata_bytes)
            bin_file.write(struct.pack('<I', metadata_length))
            
            # JSON Metadata
            bin_file.write(metadata_bytes)
            
            # Intel HEX Firmware
            with open(firmware_hex, 'rb') as hex_file:
                bin_file.write(hex_file.read())
                
        print(f"📦 Created binary package:")
        print(f"   Magic Header: FLFW\\0")
        print(f"   Metadata Length: {metadata_length} bytes")
        print(f"   JSON Metadata: {len(json_metadata)} characters")
        print(f"   Firmware Size: {firmware_hex.stat().st_size} bytes")
        print(f"   Total Package: {bin_path.stat().st_size} bytes")
            
    except Exception as e:
        print(f"❌ Error creating binary package: {e}")
        raise

# Register the post-build hook for the main build target
env.AddPostAction("buildprog", post_build)

print("📝 Post-build script loaded for buildprog target")
