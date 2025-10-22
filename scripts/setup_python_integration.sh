#!/bin/bash
# Setup script for Ladybird Python integration

# Exit on error
set -e

# Check if Python development headers are installed
check_python_dev() {
    echo "Checking for Python development headers..."
    if command -v python3-config >/dev/null 2>&1; then
        echo "Found Python development headers"
        return 0
    else
        echo "Python development headers not found"
        return 1
    fi
}

# Install Python development headers on various platforms
install_python_dev() {
    if [ -f /etc/debian_version ]; then
        echo "Debian/Ubuntu detected"
        sudo apt-get update
        sudo apt-get install -y python3-dev
    elif [ -f /etc/redhat-release ]; then
        echo "Red Hat/CentOS/Fedora detected"
        sudo dnf install -y python3-devel
    elif [ -f /etc/arch-release ]; then
        echo "Arch Linux detected"
        sudo pacman -S --needed python
    elif [ "$(uname)" == "Darwin" ]; then
        echo "macOS detected"
        if command -v brew >/dev/null 2>&1; then
            brew install python
        else
            echo "Homebrew not found. Please install it first."
            exit 1
        fi
    else
        echo "Unsupported platform. Please install Python development headers manually."
        exit 1
    fi
}

# Apply patches
apply_patches() {
    echo "Applying patches for Python integration..."
    
    # Apply Document.h patch
    if [ -f "Libraries/LibWeb/DOM/Document.h.patch" ]; then
        patch -p1 < Libraries/LibWeb/DOM/Document.h.patch || {
            echo "Failed to apply Document.h patch"
            return 1
        }
    fi
    
    # Apply CMakeLists.txt patch
    if [ -f "Libraries/LibWeb/CMakeLists.txt.patch" ]; then
        patch -p1 < Libraries/LibWeb/CMakeLists.txt.patch || {
            echo "Failed to apply CMakeLists.txt patch"
            return 1
        }
    fi
    
    # Apply fixed files
    for file in $(find . -name "*.fix"); do
        dest_file="${file%.fix}"
        echo "Applying fix: $dest_file"
        cp "$file" "$dest_file" || {
            echo "Failed to apply fix for $dest_file"
            return 1
        }
    done
    
    echo "All patches applied successfully"
    return 0
}

# Configure CMake with Python support
configure_cmake() {
    echo "Configuring CMake with Python support..."
    
    # Create build directory if it doesn't exist
    mkdir -p Build
    cd Build
    
    # Configure CMake with Python support
    cmake -DENABLE_PYTHON=ON ..
    
    echo "CMake configuration complete"
    return 0
}

# Main script
main() {
    echo "Setting up Ladybird Python integration..."
    
    # Check if Python development headers are installed
    if ! check_python_dev; then
        echo "Python development headers are required. Installing..."
        if ! install_python_dev; then
            echo "Failed to install Python development headers"
            exit 1
        fi
    fi
    
    # Apply patches
    if ! apply_patches; then
        echo "Failed to apply patches"
        exit 1
    fi
    
    # Configure CMake
    if ! configure_cmake; then
        echo "Failed to configure CMake"
        exit 1
    fi
    
    echo "Ladybird Python integration setup complete!"
    echo "You can now build Ladybird with Python support by running 'make -C Build'"
    
    return 0
}

# Run main function
main "$@"
