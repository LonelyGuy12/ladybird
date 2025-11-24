/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/HTML/Scripting/PythonPackageManager.h>
#include <LibWeb/HTML/Scripting/PythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/Fetch/Infrastructure/HTTP/Requests.h>
#include <LibWeb/Fetch/Infrastructure/HTTP/Responses.h>
#include <LibWeb/Fetch/Fetching/Fetching.h>
#include <LibWeb/Bindings/MainThreadVM.h>
#include <AK/Debug.h>
#include <AK/LexicalPath.h>
#include <AK/StringBuilder.h>
#include <AK/StringView.h>
#include <AK/GenericLexer.h>
#include <Python.h>

// System includes for package installation
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace Web::HTML {

static PythonPackageManager* s_the = nullptr;

PythonPackageManager& PythonPackageManager::the()
{
    if (!s_the)
        s_the = new PythonPackageManager;
    return *s_the;
}

ErrorOr<void> PythonPackageManager::initialize()
{
    if (m_initialized)
        return {};
    
    dbgln("🐍 PythonPackageManager: Initializing package manager");
    
    // Initialize the package installation directory
    auto package_path = get_package_install_path();
    dbgln("🐍 PythonPackageManager: Package installation path: {}", package_path);
    
    // Set up Python path to include our package directory
    TRY(setup_python_path());
    
    m_initialized = true;
    return {};
}

ErrorOr<Optional<String>> PythonPackageManager::find_requirements_file(URL::URL const& document_origin)
{
    dbgln("🐍 PythonPackageManager: Looking for requirements.txt at origin: {}", document_origin.serialize());
    
    // For local files, we can check if the file exists and read it
    if (document_origin.scheme() == "file"sv) {
        // For local files, construct the path to requirements.txt in the same directory
        auto path = document_origin.file_path();
        auto lexical_path = LexicalPath(path);
        auto dir_path = lexical_path.dirname();
        auto requirements_path = String::formatted("{}/requirements.txt", dir_path);
        
        dbgln("🐍 PythonPackageManager: Checking for local requirements.txt at: {}", requirements_path);
        
        auto requirements_path_byte_string = requirements_path.to_byte_string();
        struct stat buffer;
        if (stat(requirements_path_byte_string.characters(), &buffer) == 0) {
            // File exists, read it
            FILE* file = fopen(requirements_path_byte_string.characters(), "r");
            if (file) {
                // Get file size
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);
                
                // Allocate buffer and read file
                char* buffer = (char*)malloc(file_size + 1);
                if (buffer) {
                    size_t bytes_read = fread(buffer, 1, file_size, file);
                    buffer[bytes_read] = '\0';
                    
                    auto content_result = String::from_utf8({ buffer, bytes_read });
                    free(buffer);
                    fclose(file);
                    
                    if (content_result.is_error()) {
                        dbgln("🐍 PythonPackageManager: Failed to create string from file content");
                        return content_result.release_error();
                    }
                    
                    String content = content_result.release_value();
                    dbgln("🐍 PythonPackageManager: Successfully read local requirements.txt ({} bytes)", bytes_read);
                    dbgln("🐍 PythonPackageManager: Content: {}", content);
                    return Optional<String>(content);
                }
                fclose(file);
            }
        } else {
            dbgln("🐍 PythonPackageManager: No local requirements.txt found at: {}", requirements_path);
        }
    }
    
    // For HTTP origins, we would need to fetch the file
    // TODO: Implement HTTP fetching for remote requirements.txt files
    
    return Optional<String> {};
}

ErrorOr<Vector<PythonPackage>> PythonPackageManager::parse_requirements(String const& content, URL::URL const& document_origin)
{
    Vector<PythonPackage> packages;
    
    // Parse the requirements.txt file format
    GenericLexer lexer(content);
    
    while (!lexer.is_eof()) {
        // Skip whitespace and empty lines
        lexer.ignore_while([](char ch) { return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'; });
        
        if (lexer.is_eof())
            break;
        
        // Skip comments
        if (lexer.peek() == '#') {
            lexer.ignore_until([](char ch) { return ch == '\n' || ch == '\r'; });
            continue;
        }
        
        // Read package name and version
        StringBuilder package_name_builder;
        StringBuilder version_builder;
        
        // Read package name (until ==, >=, <=, >, <, ~=, or end of line)
        while (!lexer.is_eof() && lexer.peek() != '\n' && lexer.peek() != '\r' && lexer.peek() != '#' 
               && !(lexer.peek() == '=' && lexer.peek(1) == '=') 
               && !(lexer.peek() == '>' && lexer.peek(1) == '=') 
               && !(lexer.peek() == '<' && lexer.peek(1) == '=') 
               && !(lexer.peek() == '~' && lexer.peek(1) == '=') 
               && lexer.peek() != '>' && lexer.peek() != '<') {
            package_name_builder.append(lexer.consume());
        }
        
        auto package_name_result = package_name_builder.to_string();
        if (package_name_result.is_error())
            return package_name_result.release_error();
        auto package_name_trimmed = package_name_result.release_value().trim_whitespace();
        if (package_name_trimmed.is_error())
            return package_name_trimmed.release_error();
        String package_name = package_name_trimmed.release_value();
        
        if (package_name.is_empty())
            continue;
        
        // Skip whitespace
        lexer.ignore_while([](char ch) { return ch == ' ' || ch == '\t'; });
        
        // Check for version specifier
        Optional<String> version;
        if (!lexer.is_eof() && (lexer.peek() == '=' || lexer.peek() == '>' || lexer.peek() == '<' || lexer.peek() == '~')) {
            // Read the version specifier
            StringBuilder version_spec_builder;
            while (!lexer.is_eof() && lexer.peek() != '\n' && lexer.peek() != '\r' && lexer.peek() != '#') {
                version_spec_builder.append(lexer.consume());
            }
            auto version_spec_result = version_spec_builder.to_string();
            if (version_spec_result.is_error())
                return version_spec_result.release_error();
            auto version_spec_trimmed = version_spec_result.release_value().trim_whitespace();
            if (version_spec_trimmed.is_error())
                return version_spec_trimmed.release_error();
            version = version_spec_trimmed.release_value();
        }
        
        // Skip to end of line
        lexer.ignore_until([](char ch) { return ch == '\n' || ch == '\r'; });
        
        // Add package to list
        PythonPackage package {
            .name = package_name,
            .version = version,
            .origin = document_origin.serialize(URL::ExcludeFragment::Yes)
        };
        
        packages.append(package);
        dbgln("🐍 PythonPackageManager: Parsed package requirement: {}{}", package.name, version.has_value() ? String::formatted(" {}", *version) : String(""_string));
    }
    
    return packages;
}

ErrorOr<void> PythonPackageManager::install_packages(Vector<PythonPackage> const& packages)
{
    if (packages.is_empty())
        return {};
    
    dbgln("🐍 PythonPackageManager: Installing {} packages", packages.size());
    
    // Get the origin for caching purposes (assume all packages from same origin)
    String origin = packages.first().origin;
    
    // Check which packages need to be installed
    Vector<PythonPackage> packages_to_install;
    for (auto const& package : packages) {
        if (!is_package_installed(package)) {
            packages_to_install.append(package);
        } else {
            dbgln("🐍 PythonPackageManager: Package {} already installed, skipping", package.name);
        }
    }
    
    if (packages_to_install.is_empty()) {
        dbgln("🐍 PythonPackageManager: All packages already installed");
        return {};
    }
    
    dbgln("🐍 PythonPackageManager: Installing {} new packages", packages_to_install.size());
    
    // Create the package installation directory if it doesn't exist
    String package_install_path = get_package_install_path();
    auto package_install_path_byte_string = package_install_path.to_byte_string();
    if (mkdir(package_install_path_byte_string.characters(), 0755) == -1 && errno != EEXIST) {
        dbgln("🐍 PythonPackageManager: Failed to create package installation directory: {}", strerror(errno));
        return Error::from_string_literal("Failed to create package installation directory");
    }
    
    // Install each package using pip
    for (auto const& package : packages_to_install) {
        StringBuilder command_builder;
        command_builder.append("pip install --target "sv);
        command_builder.append(package_install_path);
        command_builder.append(" "sv);
        command_builder.append(package.name);
        
        if (package.version.has_value()) {
            command_builder.append("=="sv);
            command_builder.append(*package.version);
        }
        
        auto command_result = command_builder.to_string();
        if (command_result.is_error()) {
            dbgln("🐍 PythonPackageManager: Failed to build command string");
            return command_result.release_error();
        }
        String command = command_result.release_value();
        dbgln("🐍 PythonPackageManager: Running command: {}", command);
        
        // Execute the pip install command
        auto command_byte_string = command.to_byte_string();
        int result = system(command_byte_string.characters());
        
        if (result == 0) {
            dbgln("🐍 PythonPackageManager: Successfully installed package {}", package.name);
        } else {
            dbgln("🐍 PythonPackageManager: Failed to install package {} (exit code: {})", package.name, result);
            // Continue with other packages even if one fails
        }
    }
    
    // Update our cache of installed packages
    m_installed_packages.set(origin, packages);
    
    return {};
}

bool PythonPackageManager::is_package_installed(PythonPackage const& package) const
{
    // Check if this package is in our installed packages cache
    auto it = m_installed_packages.find(package.origin);
    if (it == m_installed_packages.end())
        return false;
    
    // Look for the package in the origin's package list
    for (auto const& installed_package : it->value) {
        if (installed_package.name == package.name) {
            // If version is specified, check if it matches
            if (package.version.has_value()) {
                return installed_package.version.has_value() && installed_package.version == package.version;
            }
            return true;
        }
    }
    
    return false;
}

String PythonPackageManager::get_package_install_path() const
{
    // Return a path for package installation
    // In a real implementation, this would be a directory within the browser's data directory
    return String("/tmp/ladybird_python_packages"_string);
}

ErrorOr<void> PythonPackageManager::setup_python_path()
{
    dbgln("🐍 PythonPackageManager: Setting up Python path");
    
    // Get the current Python path
    PyObject* sys_module = PyImport_ImportModule("sys");
    if (!sys_module) {
        dbgln("🐍 PythonPackageManager: Failed to import sys module");
        return Error::from_string_literal("Failed to import sys module");
    }
    
    PyObject* path_list = PyObject_GetAttrString(sys_module, "path");
    if (!path_list) {
        Py_DECREF(sys_module);
        dbgln("🐍 PythonPackageManager: Failed to get sys.path");
        return Error::from_string_literal("Failed to get sys.path");
    }
    
    // Add our package installation directory to the path
    String package_path = get_package_install_path();
    auto package_path_byte_string = package_path.to_byte_string();
    PyObject* path_string = PyUnicode_FromString(package_path_byte_string.characters());
    if (path_string) {
        PyList_Append(path_list, path_string);
        Py_DECREF(path_string);
        dbgln("🐍 PythonPackageManager: Added {} to Python path", package_path);
    }
    
    Py_DECREF(path_list);
    Py_DECREF(sys_module);
    
    return {};
}

void PythonPackageManager::clear_cache_for_origin(URL::URL const& origin)
{
    String origin_key = origin.serialize(URL::ExcludeFragment::Yes);
    m_installed_packages.remove(origin_key);
    dbgln("🐍 PythonPackageManager: Cleared cache for origin: {}", origin_key);
}

}