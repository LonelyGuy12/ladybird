/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Debug.h>
#include <AK/GenericLexer.h>
#include <AK/LexicalPath.h>
#include <AK/StringBuilder.h>
#include <AK/StringView.h>
#include <LibWeb/Bindings/MainThreadVM.h>
#include <LibWeb/DOM/Document.h>
#include <LibWeb/Fetch/Fetching/Fetching.h>
#include <LibWeb/Fetch/Infrastructure/HTTP/Requests.h>
#include <LibWeb/Fetch/Infrastructure/HTTP/Responses.h>
#include <LibWeb/HTML/Scripting/PythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonPackageManager.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <Python.h>

// System includes for package installation
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace Web::HTML {

static PythonPackageManager *s_the = nullptr;

#ifdef __APPLE__
// Helper to get app bundle path on macOS
static Optional<String> get_app_bundle_path() {
  char exe_path[PATH_MAX];
  uint32_t size = sizeof(exe_path);
  if (_NSGetExecutablePath(exe_path, &size) == 0) {
    // exe_path is like: /path/to/Ladybird.app/Contents/MacOS/Ladybird
    // Check if we're in an app bundle
    auto path_str_result = String::from_utf8({exe_path, strlen(exe_path)});
    if (path_str_result.is_error())
      return {};

    String path_str = path_str_result.release_value();
    if (path_str.contains(".app/Contents/MacOS"sv)) {
      // Extract bundle path by going up to .app
      auto app_index = path_str.find_byte_offset(".app/Contents/MacOS"sv);
      if (app_index.has_value()) {
        auto bundle_path_result = path_str.substring_from_byte_offset(
            0, app_index.value() + 4); // Include ".app"
        if (!bundle_path_result.is_error())
          return bundle_path_result.release_value();
      }
    }
  }
  return {};
}
#endif

PythonPackageManager &PythonPackageManager::the() {
  if (!s_the)
    s_the = new PythonPackageManager;
  return *s_the;
}

String PythonPackageManager::get_venv_base_path() const {
#ifdef __APPLE__
  // On macOS, check if we're in an app bundle
  auto bundle_path = get_app_bundle_path();
  if (bundle_path.has_value()) {
    auto venv_path_result = String::formatted(
        "{}/Contents/Resources/python_venv", bundle_path.value());
    if (!venv_path_result.is_error()) {
      dbgln("🐍 PythonPackageManager: Using bundle venv at: {}",
            venv_path_result.value());
      return venv_path_result.release_value();
    }
  }
  // Fall through to temp path for development builds
#endif

  // Development builds and other platforms use temp directory
  return String("/tmp/ladybird_python_venv"_string);
}

String PythonPackageManager::get_package_install_path() const {
  // Dynamically detect Python version to construct the correct site-packages
  // path Extract major.minor from Py_GetVersion() which returns something like
  // "3.14.0 (...)"
  char const *version_str = Py_GetVersion();

  String venv_base = get_venv_base_path();

  // Parse "3.14.0" to get "3.14"
  int major = 0, minor = 0;
  if (sscanf(version_str, "%d.%d", &major, &minor) >= 2) {
    auto path_result = String::formatted("{}/lib/python{}.{}/site-packages",
                                         venv_base, major, minor);
    if (!path_result.is_error()) {
      return path_result.release_value();
    }
  }

  // Fallback to python3.14 if parsing fails
  dbgln("🐍 PythonPackageManager: Warning - failed to parse Python version, "
        "using python3.14 as fallback");
  auto fallback_result =
      String::formatted("{}/lib/python3.14/site-packages", venv_base);
  if (!fallback_result.is_error())
    return fallback_result.release_value();

  return String(
      "/tmp/ladybird_python_venv/lib/python3.14/site-packages"_string);
}

ErrorOr<void> PythonPackageManager::initialize() {
  if (m_initialized)
    return {};

  dbgln("🐍 PythonPackageManager: Initializing package manager");

  // Create a virtual environment for package isolation
  String venv_path = get_venv_base_path();
  auto venv_path_byte_string = venv_path.to_byte_string();

  dbgln("🐍 PythonPackageManager: Using venv path: {}", venv_path);

  // Check if virtual environment already exists
  struct stat buffer;
  if (stat(venv_path_byte_string.characters(), &buffer) != 0) {
    // Virtual environment doesn't exist, create it
    dbgln("🐍 PythonPackageManager: Creating virtual environment at {}",
          venv_path);

    // Use bundled Python if available, otherwise system python3
    String python_executable = "python3"_string;
#ifdef __APPLE__
    auto bundle_path = get_app_bundle_path();
    if (bundle_path.has_value()) {
      auto bundled_python_result = String::formatted(
          "{}/Contents/Resources/bundled_python/Versions/3.14/bin/python3.14",
          bundle_path.value());
      if (!bundled_python_result.is_error()) {
        String bundled_python = bundled_python_result.release_value();
        auto bundled_python_bytes = bundled_python.to_byte_string();
        // Check if bundled Python exists
        if (stat(bundled_python_bytes.characters(), &buffer) == 0) {
          python_executable = bundled_python;
          dbgln("🐍 PythonPackageManager: Using bundled Python: {}",
                python_executable);
        }
      }
    }
#endif

    auto command_result =
        String::formatted("{} -m venv {}", python_executable, venv_path);
    if (command_result.is_error()) {
      dbgln("🐍 PythonPackageManager: Failed to format command string");
      return command_result.release_error();
    }
    String command = command_result.release_value();
    auto command_byte_string = command.to_byte_string();
    int result = system(command_byte_string.characters());

    if (result != 0) {
      dbgln("🐍 PythonPackageManager: Failed to create virtual environment");
      return Error::from_string_literal("Failed to create virtual environment");
    }
  } else {
    dbgln("🐍 PythonPackageManager: Using existing virtual environment at {}",
          venv_path);
  }

  // Upgrade pip in the virtual environment to ensure we have the latest version
  auto venv_pip_result = String::formatted("{}/bin/pip", venv_path);
  if (venv_pip_result.is_error())
    return venv_pip_result.release_error();
  String venv_pip = venv_pip_result.release_value();
  auto upgrade_command_result =
      String::formatted("{} install --upgrade pip", venv_pip);
  if (upgrade_command_result.is_error()) {
    dbgln("🐍 PythonPackageManager: Failed to format pip upgrade command");
    return upgrade_command_result.release_error();
  }
  String upgrade_command = upgrade_command_result.release_value();
  auto upgrade_command_byte_string = upgrade_command.to_byte_string();
  // We don't care about the result of this command as it's not critical
  // But we need to handle the return value to avoid compiler warnings
  [[maybe_unused]] auto unused_result =
      system(upgrade_command_byte_string.characters());

  // Set up Python path to include our virtual environment
  TRY(setup_python_path());

  m_initialized = true;
  return {};
}

ErrorOr<void> PythonPackageManager::setup_python_path() {
  // Add our virtual environment's site-packages directory to Python's sys.path
  String package_path = get_package_install_path();
  dbgln("🐍 PythonPackageManager: Adding {} to Python path", package_path);

  // Convert to ByteString for Python C API
  ByteString package_path_byte_string = package_path.to_byte_string();

  // Get the current Python path
  PyObject *sys_path = PySys_GetObject("path");
  if (!sys_path) {
    dbgln("🐍 PythonPackageManager: Failed to get Python sys.path");
    return Error::from_string_literal("Failed to get Python sys.path");
  }

  // Add our package directory to the beginning of the path for highest priority
  PyObject *path_string =
      PyUnicode_FromString(package_path_byte_string.characters());
  if (!path_string) {
    dbgln("🐍 PythonPackageManager: Failed to create Python string for package "
          "path");
    return Error::from_string_literal(
        "Failed to create Python string for package path");
  }

  int result = PyList_Insert(sys_path, 0, path_string);
  Py_DECREF(path_string);

  if (result == -1) {
    dbgln("🐍 PythonPackageManager: Failed to insert package path into Python "
          "sys.path");
    return Error::from_string_literal(
        "Failed to insert package path into Python sys.path");
  }

  dbgln("🐍 PythonPackageManager: Successfully added package path to Python "
        "sys.path");
  return {};
}

ErrorOr<Optional<String>>
PythonPackageManager::find_requirements_file(URL::URL const &document_origin) {
  dbgln("🐍 PythonPackageManager: Looking for requirements.txt at origin: {}",
        document_origin.serialize());

  // For local files, we can check if the file exists and read it
  if (document_origin.scheme() == "file"sv) {
    // For local files, construct the path to requirements.txt in the same
    // directory
    auto path = document_origin.file_path();
    auto lexical_path = LexicalPath(path);
    auto dir_path = lexical_path.dirname();
    auto requirements_path_result =
        String::formatted("{}/requirements.txt", dir_path);

    if (requirements_path_result.is_error()) {
      dbgln("🐍 PythonPackageManager: Failed to format requirements path");
      return requirements_path_result.release_error();
    }

    String requirements_path = requirements_path_result.release_value();
    dbgln("🐍 PythonPackageManager: Checking for local requirements.txt at: {}",
          requirements_path);

    auto requirements_path_byte_string = requirements_path.to_byte_string();
    struct stat buffer;
    if (stat(requirements_path_byte_string.characters(), &buffer) == 0) {
      // File exists, read it
      FILE *file = fopen(requirements_path_byte_string.characters(), "r");
      if (file) {
        // Get file size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate buffer and read file
        char *buffer = (char *)malloc(file_size + 1);
        if (buffer) {
          size_t bytes_read = fread(buffer, 1, file_size, file);
          buffer[bytes_read] = '\0';

          auto content_result = String::from_utf8({buffer, bytes_read});
          free(buffer);
          fclose(file);

          if (content_result.is_error()) {
            dbgln("🐍 PythonPackageManager: Failed to create string from file "
                  "content");
            return content_result.release_error();
          }

          String content = content_result.release_value();
          dbgln("🐍 PythonPackageManager: Successfully read local "
                "requirements.txt ({} bytes)",
                bytes_read);
          dbgln("🐍 PythonPackageManager: Content: {}", content);
          return Optional<String>(content);
        }
        fclose(file);
      }
    } else {
      dbgln("🐍 PythonPackageManager: No local requirements.txt found at: {}",
            requirements_path);
    }
  }

  // For HTTP origins, we would need to fetch the file
  // TODO: Implement HTTP fetching for remote requirements.txt files

  return Optional<String>{};
}

ErrorOr<Vector<PythonPackage>>
PythonPackageManager::parse_requirements(String const &content,
                                         URL::URL const &document_origin) {
  Vector<PythonPackage> packages;

  // Parse the requirements.txt file format
  GenericLexer lexer(content);

  while (!lexer.is_eof()) {
    // Skip whitespace and empty lines
    lexer.ignore_while([](char ch) {
      return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
    });

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
    while (!lexer.is_eof() && lexer.peek() != '\n' && lexer.peek() != '\r' &&
           lexer.peek() != '#' &&
           !(lexer.peek() == '=' && lexer.peek(1) == '=') &&
           !(lexer.peek() == '>' && lexer.peek(1) == '=') &&
           !(lexer.peek() == '<' && lexer.peek(1) == '=') &&
           !(lexer.peek() == '~' && lexer.peek(1) == '=') &&
           lexer.peek() != '>' && lexer.peek() != '<') {
      package_name_builder.append(lexer.consume());
    }

    auto package_name_result = package_name_builder.to_string();
    if (package_name_result.is_error())
      return package_name_result.release_error();
    auto package_name_trimmed =
        package_name_result.release_value().trim_whitespace();
    if (package_name_trimmed.is_error())
      return package_name_trimmed.release_error();
    String package_name = package_name_trimmed.release_value();

    if (package_name.is_empty())
      continue;

    // Skip whitespace
    lexer.ignore_while([](char ch) { return ch == ' ' || ch == '\t'; });

    // Check for version specifier
    Optional<String> version;
    if (!lexer.is_eof() && (lexer.peek() == '=' || lexer.peek() == '>' ||
                            lexer.peek() == '<' || lexer.peek() == '~')) {
      // Read the version specifier
      StringBuilder version_spec_builder;
      while (!lexer.is_eof() && lexer.peek() != '\n' && lexer.peek() != '\r' &&
             lexer.peek() != '#') {
        version_spec_builder.append(lexer.consume());
      }
      auto version_spec_result = version_spec_builder.to_string();
      if (version_spec_result.is_error())
        return version_spec_result.release_error();
      auto version_spec_trimmed =
          version_spec_result.release_value().trim_whitespace();
      if (version_spec_trimmed.is_error())
        return version_spec_trimmed.release_error();
      version = version_spec_trimmed.release_value();
    }

    // Skip to end of line
    lexer.ignore_until([](char ch) { return ch == '\n' || ch == '\r'; });

    // Add package to list
    PythonPackage package{
        .name = package_name,
        .version = version,
        .origin = document_origin.serialize(URL::ExcludeFragment::Yes)};

    packages.append(package);
    dbgln("🐍 PythonPackageManager: Parsed package requirement: {}{}",
          package.name,
          version.has_value() ? String::formatted(" {}", *version)
                              : String(""_string));
  }

  return packages;
}

ErrorOr<void>
PythonPackageManager::install_packages(Vector<PythonPackage> const &packages) {
  if (packages.is_empty())
    return {};

  dbgln("🐍 PythonPackageManager: Installing {} packages", packages.size());

  // Get the origin for caching purposes (assume all packages from same origin)
  String origin = packages.first().origin;

  // Check which packages need to be installed
  Vector<PythonPackage> packages_to_install;
  for (auto const &package : packages) {
    if (!is_package_installed(package)) {
      packages_to_install.append(package);
    } else {
      dbgln("🐍 PythonPackageManager: Package {} already installed, skipping",
            package.name);
    }
  }

  if (packages_to_install.is_empty()) {
    dbgln("🐍 PythonPackageManager: All packages already installed");
    return {};
  }

  dbgln("🐍 PythonPackageManager: Installing {} new packages",
        packages_to_install.size());

  // Use the virtual environment's pip directly
  String venv_base = get_venv_base_path();
  auto venv_pip_result = String::formatted("{}/bin/pip", venv_base);
  if (venv_pip_result.is_error())
    return venv_pip_result.release_error();
  String venv_pip = venv_pip_result.release_value();
  // Check if pip is available in the virtual environment
  auto check_command =
      String::formatted("{} --version > /dev/null 2>&1", venv_pip);
  if (check_command.is_error()) {
    dbgln("🐍 PythonPackageManager: Failed to format check command");
    return check_command.release_error();
  }
  String check_command_str = check_command.release_value();
  auto check_command_byte_string = check_command_str.to_byte_string();
  int pip_check = system(check_command_byte_string.characters());

  if (pip_check != 0) {
    dbgln(
        "🐍 PythonPackageManager: pip is not available in virtual environment. "
        "Please ensure the virtual environment is properly created.");
    return Error::from_string_literal(
        "pip is not available in virtual environment");
  }

  // Install each package using pip in our virtual environment
  for (auto const &package : packages_to_install) {
    StringBuilder command_builder;
    command_builder.append(venv_pip);
    command_builder.append(" install --upgrade "_string);
    command_builder.append(package.name);

    if (package.version.has_value()) {
      // Fix the version specifier - remove any spaces or extra characters
      String version = *package.version;
      // Remove leading/trailing whitespace
      auto trimmed_version = version.trim_whitespace();
      if (!trimmed_version.is_error() && !trimmed_version.value().is_empty()) {
        // Check if version already has an operator
        if (!trimmed_version.value().starts_with_bytes("="sv) &&
            !trimmed_version.value().starts_with_bytes(">"sv) &&
            !trimmed_version.value().starts_with_bytes("<"sv) &&
            !trimmed_version.value().starts_with_bytes("~"sv)) {
          command_builder.append("=="sv);
        }
        command_builder.append(trimmed_version.value());
      }
    }

    // Special handling for numpy to avoid installation issues
    if (package.name == "numpy"_string) {
      // Add --no-cache-dir to avoid potential cache issues
      command_builder.append(" --no-cache-dir"_string);
      // Also add --force-reinstall to ensure a clean installation
      command_builder.append(" --force-reinstall"_string);
      // Add --no-deps to avoid dependency conflicts
      command_builder.append(" --no-deps"_string);
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
      dbgln("🐍 PythonPackageManager: Successfully installed package {}",
            package.name);
    } else {
      dbgln("🐍 PythonPackageManager: Failed to install package {} (exit code: "
            "{})",
            package.name, result);
      // Continue with other packages even if one fails
    }
  }

  // Update our cache of installed packages
  m_installed_packages.set(origin, packages);

  return {};
}

bool PythonPackageManager::is_package_installed(
    PythonPackage const &package) const {
  // Check if this package is in our installed packages cache
  auto it = m_installed_packages.find(package.origin);
  if (it == m_installed_packages.end())
    return false;

  // Look for the package in the origin's package list
  for (auto const &installed_package : it->value) {
    if (installed_package.name == package.name) {
      // If version is specified, check if it matches
      if (package.version.has_value()) {
        return installed_package.version.has_value() &&
               installed_package.version == package.version;
      }
      return true;
    }
  }

  return false;
}

void PythonPackageManager::clear_cache_for_origin(URL::URL const &origin) {
  String origin_key = origin.serialize(URL::ExcludeFragment::Yes);
  m_installed_packages.remove(origin_key);
  dbgln("🐍 PythonPackageManager: Cleared cache for origin: {}", origin_key);
}

} // namespace Web::HTML
