#!/usr/bin/env python3
"""
Version bump script for firmware
Usage: python3 bump_version.py [major|minor|patch]
"""

import re
import sys
from pathlib import Path

def bump_version(version_type):
    """Bump version in main.cpp"""
    main_cpp = Path("src/main.cpp")
    
    if not main_cpp.exists():
        print("❌ src/main.cpp not found!")
        return False
    
    # Read current content
    with open(main_cpp, 'r') as f:
        content = f.read()
    
    # Find current version
    version_pattern = r'#define\s+FIRMWARE_VERSION\s+"([^"]+)"'
    match = re.search(version_pattern, content)
    
    if not match:
        print("❌ FIRMWARE_VERSION not found in main.cpp!")
        return False
    
    current_version = match.group(1)
    print(f"📋 Current version: {current_version}")
    
    # Parse version components
    parts = current_version.split('.')
    if len(parts) != 3:
        print("❌ Version format should be X.Y.Z")
        return False
    
    major, minor, patch = map(int, parts)
    
    # Bump version based on type
    if version_type == "major":
        major += 1
        minor = 0
        patch = 0
    elif version_type == "minor":
        minor += 1
        patch = 0
    elif version_type == "patch":
        patch += 1
    else:
        print("❌ Invalid version type. Use: major, minor, or patch")
        return False
    
    new_version = f"{major}.{minor}.{patch}"
    print(f"🆕 New version: {new_version}")
    
    # Update the file
    new_content = re.sub(version_pattern, f'#define FIRMWARE_VERSION "{new_version}"', content)
    
    with open(main_cpp, 'w') as f:
        f.write(new_content)
    
    print(f"✅ Updated {main_cpp} to version {new_version}")
    
    # Also update firmware.meta if it exists
    meta_file = Path("firmware.meta")
    if meta_file.exists():
        import json
        with open(meta_file, 'r') as f:
            metadata = json.load(f)
        
        metadata["firmware"]["version"] = new_version
        
        with open(meta_file, 'w') as f:
            json.dump(metadata, f, indent=2)
        
        print(f"✅ Updated {meta_file} to version {new_version}")
    
    return True

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 bump_version.py [major|minor|patch]")
        print("Examples:")
        print("  python3 bump_version.py patch    # 1.0.0 -> 1.0.1")
        print("  python3 bump_version.py minor    # 1.0.0 -> 1.1.0")
        print("  python3 bump_version.py major    # 1.0.0 -> 2.0.0")
        sys.exit(1)
    
    version_type = sys.argv[1].lower()
    
    if version_type not in ["major", "minor", "patch"]:
        print("❌ Invalid version type. Use: major, minor, or patch")
        sys.exit(1)
    
    if bump_version(version_type):
        print(f"🎉 Version bumped successfully!")
        print(f"💡 Don't forget to commit and push: git add . && git commit -m 'Bump version' && git push")
    else:
        print("❌ Version bump failed!")
        sys.exit(1)

if __name__ == "__main__":
    main()
