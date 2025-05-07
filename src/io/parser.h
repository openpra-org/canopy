#ifndef CANOPY_IO_PARSER_H
#define CANOPY_IO_PARSER_H

/**
 * @file parser.h
 * @author Arjun Earthperson
 * @date 09/01/2023
 * @brief This file contains the definitions for functions that handle file parsing operations.
 */

#include <string>

#include <nlohmann/json.hpp>

namespace canopy::io {

/**
 * @brief Checks if a file is writable.
 * @param filepath The path to the file.
 * @return True if the file is writable, false otherwise.
 */
bool isFileWritable(const std::string &filepath);

/**
 * @brief Checks if a directory exists. If it doesn't exist, it is created.
 * @param directoryPath The path to the directory.
 * @return True if the directory is writable, false otherwise.
 */
bool isDirectoryWritable(const std::string &directoryPath);

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
bool doesFileExist(const std::string &filepath);

/**
 * @brief Reads a CSV file and stores the data in a map.
 * @tparam T The type of the data to be read.
 * @param filepath The path to the CSV file.
 * @param data A reference to a map where the data will be stored.
 */
template <typename T> void readCSV(const std::string &filepath, std::map<std::string, std::vector<T>> &data);

/**
 * @brief Reads a CSV file without headers and stores the data in a 2D vector.
 * @tparam T The type of the data to be read.
 * @param filepath The path to the CSV file.
 * @param data A reference to a 2D vector where the data will be stored.
 */
template <typename T> void readCSVRowWiseNoHeaders(const std::string &filepath, std::vector<std::vector<T>> &data);

/**
 * @brief Converts a vector of any type to a vector of strings.
 *
 * This function template takes a vector of any type and converts each element to a string.
 * The conversion is done using a stringstream and the scientific notation is used for the conversion.
 * The precision of the conversion can be specified as an optional parameter.
 *
 * @tparam T The type of the elements in the input vector. This can be any type that can be streamed into a stringstream.
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
std::vector<std::string> asStringVector(const std::vector<T> &inputVector, const int precision = 19);

/**
 * @brief Writes data to a CSV file.
 * @param filepath The path to the CSV file.
 * @param data A reference to a map containing the data to be written.
 * @param columns A vector containing the names of the columns.
 */
void writeCSV(const std::string &filepath, std::map<std::string, std::vector<std::string>> &data,
                     const std::vector<std::string> &columns);

/**
 * @brief A template function to write a matrix to a CSV file without headers.
 * @details This function writes a matrix to a CSV file. If the file is not writable, an error message is printed.
 * @tparam T The type of the matrix elements.
 * @param filepath The path to the CSV file.
 * @param data The matrix to be written to the CSV file.
 */
template <typename T>
void writeCSVMatrixNoHeaders(const std::string &filepath, std::vector<std::vector<T>> &data);

template <typename T>
void writeCSVMatrixNoHeaders(const std::string &directory, const std::string &file, std::vector<std::vector<T>> &data);

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
void readJSON(const std::string &filepath, nlohmann::json &map);

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
void writeJSON(const std::string &filepath, nlohmann::json &data);

} // namespace canopy::io

#endif // CANOPY_IO_PARSER_H
