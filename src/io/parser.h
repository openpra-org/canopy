#ifndef CANOPY_IO_PARSER_H
#define CANOPY_IO_PARSER_H

/**
 * @file parser.h
 * @author Arjun Earthperson
 * @date 09/01/2023
 * @brief This file contains the definitions for functions that handle file parsing operations.
 */

#include <boost/tokenizer.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "json.h"

namespace canopy::io {

/**
 * @brief Checks if a file is writable.
 * @param filepath The path to the file.
 * @return True if the file is writable, false otherwise.
 */
static bool isFileWritable(const std::string &filepath) {
    const std::filesystem::path path(filepath);
    std::error_code ec; // For using the non-throwing overloads of functions below.
    if (exists(path, ec)) {
        if (std::filesystem::is_directory(path, ec)) {
            std::cerr << "Provided path is a directory: " << filepath << std::endl;
            return false;
        }
        return true;
    }
    return true;
}

/**
 * @brief Checks if a directory exists. If it doesn't exist, it is created.
 * @param directoryPath The path to the directory.
 * @return True if the directory is writable, false otherwise.
 */
static bool isDirectoryWritable(const std::string &directoryPath) {
    const std::filesystem::path path(directoryPath);
    std::error_code ec; // For using the non-throwing overloads of functions below.
    if (exists(path, ec)) {
        if (!std::filesystem::is_directory(path, ec)) {
            std::cerr << "Error: Provided path is a not directory: " << directoryPath << std::endl;
            return false;
        }
        return true;
    } else {
        // Create the directory and all necessary parent directories, similar to "mkdir -p"
        if (!std::filesystem::create_directories(path, ec)) {
            std::cerr << "Error: Unable to create directory: " << directoryPath << std::endl;
            return false;
        }
    }
    return true;
}

/**
 * @brief Checks if a file exists and is not a directory.
 *
 * This function checks if a file exists at the given file path and if it is not a directory.
 * If the file does not exist or if it is a directory, an error message is displayed and the function returns false.
 * If the file exists and is not a directory, the function returns true.
 *
 * @param filepath The path to the file.
 * @return True if the file exists and is not a directory, false otherwise.
 */
static bool doesFileExist(const std::string &filepath) {
    const std::filesystem::path path(filepath);
    std::error_code ec; // For using the non-throwing overloads of functions below.
    if (!exists(path, ec)) {
        return false;
    }
    if (std::filesystem::is_directory(path, ec)) {
        std::cerr << "Error: Provided path is a directory. " << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Reads a CSV file and stores the data in a map.
 * @tparam T The type of the data to be read.
 * @param filepath The path to the CSV file.
 * @param data A reference to a map where the data will be stored.
 */
template <typename T> void readCSV(const std::string &filepath, std::map<std::string, std::vector<T>> &data) {

    // Open the CSV file
    std::ifstream inputFile(filepath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open the input CSV: " << filepath << std::endl;
        exit(1);
    }
    std::vector<std::string> columnIndices;

    // Read the header row to get column names
    std::string header;
    if (std::getline(inputFile, header)) {
        // Tokenize the header row using boost::tokenizer
        boost::char_separator<char> separator(",");
        boost::tokenizer<boost::char_separator<char>> headerTokens(header, separator);

        // Initialize the map with column names as keys
        for (const auto &columnName : headerTokens) {
            data[columnName].clear(); // Clear any existing data for this column
            columnIndices.push_back(columnName);
        }

        // Read and parse the data rows
        std::string line;
        while (std::getline(inputFile, line)) {
            // Tokenize the line using boost::tokenizer
            boost::tokenizer<boost::char_separator<char>> tokens(line, separator);

            size_t idx = 0;
            for (const auto &token : tokens) {
                try {
                    (data[columnIndices[idx++]]).push_back(boost::lexical_cast<long double>(token));
                } catch (const boost::bad_lexical_cast &ex) {
                    std::cerr << "Error: Failed to convert to long double: " << ex.what() << std::endl;
                }
            }
        }
    } else {
        std::cerr << "Error: CSV file is empty." << std::endl;
    }

    // Close the file
    inputFile.close();
}

/**
 * @brief Reads a CSV file without headers and stores the data in a 2D vector.
 * @tparam T The type of the data to be read.
 * @param filepath The path to the CSV file.
 * @param data A reference to a 2D vector where the data will be stored.
 */
template <typename T> void readCSVRowWiseNoHeaders(const std::string &filepath, std::vector<std::vector<T>> &data) {

    // Open the CSV file
    std::ifstream inputFile(filepath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open the input CSV: " << filepath << std::endl;
        exit(1);
    }

    // Read and parse the data rows
    std::string line;
    while (std::getline(inputFile, line)) {
        // Tokenize the line using boost::tokenizer
        boost::char_separator<char> separator(",");
        boost::tokenizer<boost::char_separator<char>> tokens(line, separator);

        std::vector<T> row;
        for (const auto &token : tokens) {
            try {
                row.push_back(boost::lexical_cast<T>(token));
            } catch (const boost::bad_lexical_cast &ex) {
                std::cerr << "Error: Failed to convert to type T: " << ex.what() << std::endl;
            }
        }
        data.push_back(row);
    }

    // Close the file
    inputFile.close();
}

/**
 * @brief Converts a vector of any type to a vector of strings.
 *
 * This function template takes a vector of any type and converts each element to a string.
 * The conversion is done using a stringstream and the scientific notation is used for the conversion.
 * The precision of the conversion can be specified as an optional parameter.
 *
 * @tparam T The type of the elements in the input vector. This can be any type that can be streamed into a
 * stringstream.
 * @param inputVector The vector of elements to be converted to strings.
 * @param precision The precision to be used for the conversion. This is optional and defaults to 19.
 * @return A vector of strings where each string is the string representation of the corresponding element in the input
 * vector.
 *
 * Example usage:
 * @code
 * std::vector<int> intVector = {1, 2, 3};
 * std::vector<std::string> stringVector = asStringVector(intVector);
 * @endcode
 */
template <typename T>
static std::vector<std::string> asStringVector(const std::vector<T> &inputVector, const int precision = 19) {
    std::vector<std::string> stringVector;
    stringVector.reserve(inputVector.size()); // Reserve space for efficiency

    for (const T &value : inputVector) {
        std::stringstream ss;
        ss << std::scientific << std::setprecision(static_cast<int>(precision)) << value;
        stringVector.push_back(ss.str());
    }

    return stringVector;
}

/**
 * @brief Writes data to a CSV file.
 * @param filepath The path to the CSV file.
 * @param data A reference to a map containing the data to be written.
 * @param columns A vector containing the names of the columns.
 */
static void writeCSV(const std::string &filepath, std::map<std::string, std::vector<std::string>> &data,
                     const std::vector<std::string> &columns) {
    if (!isFileWritable(filepath)) {
        std::cerr << "Error: Unable to write output CSV to path: " << filepath << std::endl;
        return;
    }

    // Open the CSV file for writing
    std::ofstream csvFile(filepath);

    // Check if the file was opened successfully
    if (!csvFile.is_open()) {
        std::cerr << "Error opening the output CSV file" << filepath << std::endl;
        return;
    }

    // Write the header row
    for (const auto &column : columns) {
        if (data.count(column)) {
            csvFile << column << ",";
        }
    }
    csvFile << "\n";

    // Find the maximum size among all value vectors to determine the number of rows
    size_t numRows = 0;
    for (const auto &pair : data) {
        size_t currentSize = pair.second.size();
        if (currentSize > numRows) {
            numRows = currentSize;
        }
    }

    // Write the data rows
    for (size_t i = 0; i < numRows; i++) {
        for (const auto &column : columns) {
            if (data.count(column)) {
                if (i < data[column].size()) {
                    csvFile << data[column][i];
                }
                csvFile << ",";
            }
        }
        csvFile << "\n";
    }

    // Close the CSV file
    csvFile.close();
}

/**
 * @brief A template function to write a matrix to a CSV file without headers.
 * @details This function writes a matrix to a CSV file. If the file is not writable, an error message is printed.
 * @tparam T The type of the matrix elements.
 * @param filepath The path to the CSV file.
 * @param data The matrix to be written to the CSV file.
 */
template <typename T>
static void writeCSVMatrixNoHeaders(const std::string &filepath, std::vector<std::vector<T>> &data) {

    if (!isFileWritable(filepath)) {
        std::cerr << "Error: Unable to write output CSV to path: " << filepath << std::endl;
        return;
    }

    // Open the CSV file for writing
    std::ofstream csvFile(filepath);

    // Check if the file was opened successfully
    if (!csvFile.is_open()) {
        std::cerr << "Error opening the output CSV file" << filepath << std::endl;
        return;
    }

    for (size_t i = 0; i < data.getRows(); i++) {
        for (size_t j = 0; j < data.getCols(); j++) {
            csvFile << data[i][j] << ((j == data.getCols() - 1) ? "\n" : ",");
        }
    }

    // Close the CSV file
    csvFile.close();
}

template <typename T>
static void writeCSVMatrixNoHeaders(const std::string &directory, const std::string &file,
                                    std::vector<std::vector<T>> &data) {
    if (!isDirectoryWritable(directory)) {
        return;
    }
    writeCSVMatrixNoHeaders(directory + "/" + file, data);
}

/**
 * @brief Reads a JSON file and stores the data in a JSON object.
 *
 * This function opens a JSON file at the given file path for reading. If the file cannot be opened,
 * an error message is displayed and the program exits. The JSON data is then read from the file and
 * stored in the provided JSON object.
 *
 * @param filepath The path to the JSON file.
 * @param map A reference to a JSON object where the data will be stored.
 */
static void readJSON(const std::string &filepath, nlohmann::json &map) {

    // Open the JSON file
    std::ifstream inputFile(filepath);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open the input JSON: " << filepath << std::endl;
        exit(1);
    }

    // read the file
    map = nlohmann::json::parse(inputFile);
}

/**
 * @brief Writes the given JSON data to a file at the specified file path.
 *
 * This function checks if the file is writable and opens it for writing. If the file cannot be opened,
 * an error message is displayed. The JSON data is then written to the file with pretty formatting
 * (indented with 4 spaces). A success message is displayed upon successful writing of the JSON data.
 *
 * @param filepath The path of the file to write the JSON data to.
 * @param data The JSON data to be written to the file.
 */
static void writeJSON(const std::string &filepath, nlohmann::json &data) {

    if (!isFileWritable(filepath)) {
        std::cerr << "Error: Unable to write output JSON to path: " << filepath << std::endl;
        return;
    }

    // Open the CSV file for writing
    std::ofstream jsonFile(filepath);

    // Check if the file was opened successfully
    if (!jsonFile.is_open()) {
        std::cerr << "Error opening the output JSON file" << filepath << std::endl;
        return;
    }

    // Write the JSON data to the file
    jsonFile << data.dump(4); // The '4' argument is for pretty formatting with 4 spaces

    std::cout << "JSON data has been written to " << filepath << std::endl;
}
} // namespace canopy::io

#endif // CANOPY_IO_PARSER_H
