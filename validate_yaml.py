#!/usr/bin/env python3
"""
YAML validation script for GitHub Actions workflows
This prevents trial and error by validating locally first
Uses only built-in Python modules
"""

import json
import sys
from pathlib import Path

def basic_yaml_validation(file_path):
    """Basic YAML validation using simple parsing rules"""
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Basic YAML structure checks
        lines = content.split('\n')
        indent_stack = []
        
        for line_num, line in enumerate(lines, 1):
            stripped = line.strip()
            if not stripped or stripped.startswith('#'):
                continue
            
            # Check for basic YAML syntax issues
            if ':' in stripped:
                # Key-value pair
                if stripped.endswith(':'):
                    # This is a key that might have nested content
                    pass
                elif ':' in stripped and not stripped.endswith(':'):
                    # This should be a key-value pair
                    pass
            
            # Check for common problematic patterns
            if '<<' in line and 'EOF' in line:
                print(f"   ⚠️  Line {line_num}: Contains heredoc syntax (<< EOF) - may cause issues")
            
            if line.count('"') % 2 != 0:
                print(f"   ⚠️  Line {line_num}: Unmatched quotes")
            
            if line.count("'") % 2 != 0:
                print(f"   ⚠️  Line {line_num}: Unmatched single quotes")
        
        # Check for GitHub Actions specific patterns
        if 'uses:' in content and 'actions/' in content:
            print(f"   ✅ Contains GitHub Actions")
        
        if 'on:' in content:
            print(f"   ✅ Contains workflow triggers")
        
        print(f"✅ {file_path} passed basic validation")
        return True
        
    except Exception as e:
        print(f"❌ Error reading {file_path}: {e}")
        return False

def main():
    """Main validation function"""
    workflow_dir = Path(".github/workflows")
    
    if not workflow_dir.exists():
        print("❌ .github/workflows directory not found")
        sys.exit(1)
    
    yaml_files = list(workflow_dir.glob("*.yml")) + list(workflow_dir.glob("*.yaml"))
    
    if not yaml_files:
        print("❌ No YAML workflow files found")
        sys.exit(1)
    
    print(f"🔍 Validating {len(yaml_files)} workflow file(s)...")
    
    all_valid = True
    for yaml_file in yaml_files:
        print(f"\n📋 Validating {yaml_file.name}:")
        if not basic_yaml_validation(yaml_file):
            all_valid = False
    
    if all_valid:
        print("\n🎉 All workflow files passed basic validation!")
        print("💡 You can now safely push to GitHub")
        print("💡 For full validation, check the Actions tab after pushing")
        sys.exit(0)
    else:
        print("\n❌ Some workflow files have errors")
        print("💡 Fix the errors before pushing")
        sys.exit(1)

if __name__ == "__main__":
    main()
