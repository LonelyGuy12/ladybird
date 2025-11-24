/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/HashMap.h>
#include <AK/Optional.h>
#include <AK/String.h>
#include <AK/Vector.h>
#include <LibURL/URL.h>
#include <LibWeb/Forward.h>

namespace Web::HTML {

struct PythonPackage {
    String name;
    Optional<String> version;
    String origin; // The origin domain this package is associated with
};

class PythonPackageManager {
public:
    static PythonPackageManager& the();

    // Initialize the package manager
    ErrorOr<void> initialize();
    
    // Check if a requirements.txt file exists for the given document origin
    ErrorOr<Optional<String>> find_requirements_file(URL::URL const& document_origin);
    
    // Parse a requirements.txt file content
    ErrorOr<Vector<PythonPackage>> parse_requirements(String const& content, URL::URL const& document_origin);
    
    // Install packages if not already installed
    ErrorOr<void> install_packages(Vector<PythonPackage> const& packages);
    
    // Check if a package is already installed
    bool is_package_installed(PythonPackage const& package) const;
    
    // Get the installation path for packages
    String get_package_install_path() const;
    
    // Set up the Python path to include installed packages
    ErrorOr<void> setup_python_path();
    
    // Clear cache for a specific origin
    void clear_cache_for_origin(URL::URL const& origin);

private:
    PythonPackageManager() = default;
    
    // Cache of installed packages
    HashMap<String, Vector<PythonPackage>> m_installed_packages;
    
    // Cache of package installation paths
    HashMap<String, String> m_package_paths;
    
    // Flag to track initialization
    bool m_initialized { false };
};

}