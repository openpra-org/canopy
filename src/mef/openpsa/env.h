/// @file
/// environment variables.
///
/// All paths are absolute, canonical, and POSIX (with '/' separator).
///
/// @pre The system follows the Filesystem Hierarchy Standard.

#pragma once

#include <stdexcept>
#include <string>

namespace mef::openpsa::env {

/// @returns The location of the RELAX NG schema for project files.
const std::string& project_schema();

/// @returns The location of the RELAX NG schema for input files.
const std::string& input_schema();

/// @returns The location of the RELAX NG schema for output report files.
const std::string& report_schema();

/// @returns The path to the installation directory.
const std::string& install_dir();

const std::string& project_schema() {
    static const std::string schema_path = install_dir() + "/share/scram/project.rng";
    return schema_path;
}

const std::string& input_schema() {
    static const std::string schema_path = install_dir() + "/share/scram/input.rng";
    return schema_path;
}

const std::string& report_schema() {
    static const std::string schema_path = install_dir() + "/share/scram/report.rng";
    return schema_path;
}

const std::string& install_dir() {
    static const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Environment variable HOME is not set.");
    }
    static const std::string install_path = std::string(home) + "/.local";
    return install_path;
}
}  // namespace scram::env
