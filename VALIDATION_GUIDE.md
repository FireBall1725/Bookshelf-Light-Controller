# GitHub Actions Validation Guide

## 🚫 **No More Trial and Error!**

This guide shows you how to validate GitHub Actions workflows locally before pushing, preventing the frustrating cycle of push → fail → fix → push → fail.

## 🔍 **Local Validation Tools**

### 1. **Basic YAML Validation Script**
```bash
python3 validate_yaml.py
```

This script checks for:
- Basic YAML structure
- Common syntax issues
- GitHub Actions patterns
- Quote matching
- Problematic shell script patterns

### 2. **GitHub's Built-in Validation**
- **Actions Tab**: Shows real-time validation results
- **Workflow Editor**: Provides inline error checking
- **Marketplace**: Validates action versions and compatibility

## 📋 **Validation Checklist**

### ✅ **Before Pushing:**
1. **Run local validation**: `python3 validate_yaml.py`
2. **Check action versions**: Use latest stable versions
3. **Simplify shell scripts**: Avoid complex embedded scripts
4. **Test indentation**: Ensure consistent spacing
5. **Validate syntax**: Use proper YAML structure

### ✅ **Common Issues to Avoid:**
- **Complex shell scripts** inside `run:` sections
- **Heredoc syntax** (`<< EOF`) in YAML
- **Deprecated actions** (check GitHub's changelog)
- **Unmatched quotes** in shell commands
- **Inconsistent indentation**

## 🛠️ **Best Practices**

### 1. **Keep It Simple**
```yaml
# ❌ Complex - Hard to validate
run: |
  complex_script=$(grep 'pattern' file | sed 's/.*"\([^"]*\)".*/\1/')
  echo "Result: $complex_script"

# ✅ Simple - Easy to validate
run: |
  echo "Simple command"
  cp source dest
```

### 2. **Use Built-in GitHub Features**
```yaml
# ❌ Custom versioning logic
run: |
  VERSION=$(grep 'VERSION' file | cut -d'"' -f2)
  echo "Version: $VERSION"

# ✅ Use GitHub context
name: Build ${{ github.sha }}
```

### 3. **Validate Action Versions**
```yaml
# ❌ Deprecated
uses: actions/upload-artifact@v3

# ✅ Current
uses: actions/upload-artifact@v4
```

## 🔧 **Troubleshooting**

### **YAML Syntax Errors:**
1. **Check indentation** - Use spaces, not tabs
2. **Validate quotes** - Ensure proper matching
3. **Simplify scripts** - Break complex operations into steps
4. **Use validation tools** - Don't guess!

### **Action Deprecation:**
1. **Check GitHub changelog** for deprecation notices
2. **Update to latest versions** regularly
3. **Test locally** before pushing
4. **Use marketplace** to find current actions

## 📚 **Resources**

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitHub Actions Marketplace](https://github.com/marketplace?type=actions)
- [YAML Syntax Guide](https://yaml.org/spec/)
- [GitHub Changelog](https://github.blog/changelog/)

## 🎯 **Success Pattern**

1. **Write simple workflow**
2. **Validate locally** with `validate_yaml.py`
3. **Push to GitHub**
4. **Check Actions tab** for success
5. **No more trial and error!**

---

**Remember**: A simple, working workflow is better than a complex, broken one!
