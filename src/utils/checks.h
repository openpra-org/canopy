#pragma once

/**
 * @file checks.h
 * @brief This file contains the declaration of functions for checking the validity of input parameters.
 * @date 08/30/2023
 * @author Arjun Earthperson
 */


#include <boost/program_options.hpp>
#include <iostream>
#include <iomanip>

#include "io/json.h"

namespace canopy::utils {

/**
 * @brief Checks if the vector stored in the boost::program_options::variables_map under the given key is unfilled.
 *
 * This function attempts to retrieve a vector of type T from the variables_map using the provided key.
 * It then checks if the size of the retrieved vector is less than the expected size, indicating that the vector is
 * unfilled. If any exception occurs during this process (e.g., the key does not exist in the map, or the value
 * associated with the key is not a vector of type T), the function catches the exception and returns true, indicating
 * that the vector is considered as unfilled.
 *
 * @tparam T The type of the elements in the vector.
 * @param values The boost::program_options::variables_map from which to retrieve the vector.
 * @param key The key associated with the vector in the variables_map.
 * @param expectedSize The expected size of the vector.
 * @return true if the vector is unfilled (i.e., its size is less than the expected size or an exception occurred),
 * false otherwise.
 */
template <typename T>
static bool isUnfilledVector(boost::program_options::variables_map &values, const std::string &key,
                             const size_t expectedSize) {
    try {
        return values[key].as<std::vector<T>>().size() < expectedSize;
    } catch (...) {
        return true;
    }
}

/**
 * @brief Checks if the input value is a positive number.
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @return true If the input value is not a positive number.
 * @return false If the input value is a positive number.
 */
template <typename T> static bool failsPositiveNumberCheck(T value) {
    if (value <= 0) {
        std::cerr << "Error: "
                  << "Input should be a positive number\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the input value is a non-negative number.
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @return true If the input value is a negative number.
 * @return false If the input value is a non-negative number.
 */
template <typename T> static bool failsNonNegativeNumberCheck(T value) {
    if (value < 0) {
        std::cerr << "Error: "
                  << "Input should be a non-negative number\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the absolute value of the input is greater than or equal to 1.
 *
 * @param value The input value to be checked.
 * @return true If the absolute value of the input is greater than or equal to 1.
 * @return false If the absolute value of the input is less than 1.
 */
static bool absoluteValueFailsGreaterThan1Check(long double value) {
    const long double max = 1.0f;
    if (abs(value) >= max) {
        std::cerr << "Error: "
                  << "abs(x) is greater than " << std::setprecision(19) << max << "\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the input value is a positive natural number.
 *
 * @param value The input value to be checked.
 * @return true If the input value is not a positive natural number.
 * @return false If the input value is a positive natural number.
 */
 template <typename T>
static bool failsNaturalNumberCheck(T value) {
    const T min = 0;
    auto error = false;
    if (value <= min) {
        std::cerr << "Error: "
                  << "Input should be a positive number\n";
        error = true;
    }
    if (std::ceil(value) != std::floor(value)) {
        std::cerr << "Error: "
                  << "Input should be a natural number\n";
        error = true;
    }

    if (value == 0) {
        std::cerr << "Error: "
                  << "Input cannot be zero\n";
        error = true;
    }

    return error;
}

/**
 * @brief Checks if the input value is a whole number.
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @return true If the input value is not a whole number.
 * @return false If the input value is a whole number.
 */
template <typename T> static bool failsWholeNumberCheck(T value) {

    const long double min = 0;
    auto error = false;

    if (value < min) {
        std::cerr << "Error: "
                  << "Input should be a non-negative number\n";
        error = true;
    }
    if (std::ceil(value) != std::floor(value)) {
        std::cerr << "Error: "
                  << "Input should be a natural number\n";
        error = true;
    }

    return error;
}

/**
 * @brief Checks if the input value is within the specified range (inclusive).
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return true If the input value is outside the specified range.
 * @return false If the input value is within the specified range.
 */
template <typename T> static bool failsInclusiveRangeCheck(T value, T min, T max) {
    if (value < min || value > max) {
        std::cerr << "Error: Input should be within the range [" << min << ", " << max << "]\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the input value is within the specified range (exclusive).
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return true If the input value is outside the specified range.
 * @return false If the input value is within the specified range.
 */
template <typename T> static bool failsExclusiveRangeCheck(T value, T min, T max) {
    if (value <= min || value > max) {
        std::cerr << "Error: Input should be within the range (" << min << ", " << max << ")\n";
        return true;
    }
    return false;
}

/**
 * @brief Checks if the input value is a valid probability (between 0 and 1 inclusive).
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @return true If the input value is not a valid probability.
 * @return false If the input value is a valid probability.
 */
template <typename T> static bool failsProbabilityCheck(T value) { return failsInclusiveRangeCheck(value, 0, 1); }

/**
 * @brief Checks if the input value is a power of 2.  Uses bitwise AND to check if there is only one '1' bit in the
 * binary representation of the number. If the result is 0, then it's a power of 2. If not, it's not a power of 2.
 *
 * @tparam T The type of the input value.
 * @param num The input value to be checked.
 * @return true If the input value is a power of 2.
 * @return false If the input value is not a power of 2.
 */
template <typename T> static bool failsPowerOf2Check(T num) {
    if (num <= 0) {
        return true;  // 0 and negative numbers are not powers of 2
    }

    return (num & (num - 1)) != 0;
}

/**
 * @brief Converts a string to a long double number.
 *
 * This function attempts to convert a given string to a long double number.
 * If the string cannot be converted to a number (invalid argument), or if the
 * number is out of range for a double, an exception is thrown and an error message
 * is printed to the standard error stream.
 *
 * @param input The string to be converted to a long double number.
 * @return The converted long double number.
 * @throws std::invalid_argument If the string cannot be converted to a number.
 * @throws std::out_of_range If the number is out of range for a double.
 */
static long double asNumber(const std::string &input) {
    try {
        return std::stod(input);
    } catch (const std::invalid_argument &) {
        std::cerr << "Argument is invalid\n";
        throw;
    } catch (const std::out_of_range &) {
        std::cerr << "Argument is out of range for a double\n";
        throw;
    } catch (...) {
        throw;
    }
}

/**
 * @brief Converts all characters in a given string to lowercase.
 *
 * @param mixedCaseStr A reference to the input string with mixed case characters.
 */
static void toLowerCase(std::string &mixedCaseStr) {
    for (char &c : mixedCaseStr) {
        c = static_cast<char>(std::tolower(c));
    }
}

/**
 * @brief Removes all spaces from a given string.
 *
 * @param spaceyStr A reference to the input string containing spaces.
 */
static void stripSpaces(std::string &spaceyStr) {
    spaceyStr.erase(std::remove_if(spaceyStr.begin(), spaceyStr.end(), ::isspace), spaceyStr.end());
}

/**
 * @brief Converts a given string to a boolean value based on its content.
 *
 * The function accepts "yes", "y", "no", and "n" as valid inputs (case-insensitive).
 * If the input is not one of these values, an exception is thrown.
 *
 * @param input A reference to the input string.
 * @return true If the input string is "yes" or "y".
 * @return false If the input string is "no" or "n".
 * @throws std::exception If the input string is not one of the valid values.
 */
static bool asYesOrNo(std::string input) {
    toLowerCase(input);
    stripSpaces(input);
    if (input == "y" || input == "yes") {
        return true;
    }

    if (input == "n" || input == "no") {
        return false;
    }

    std::cerr << "Unknown value, please specify one of: [yes,y,no,n]\n";
    throw std::exception();
}

/**
 * @brief Checks if the input value fails any of the provided checks.
 *
 * This function iterates through a vector of check functions and calls each of them with the input value.
 * If any of the check functions return true, the function returns true, indicating that the value fails
 * at least one of the checks. If none of the check functions return true, the function returns false,
 * indicating that the value passes all the checks.
 *
 * @tparam T The type of the input value.
 * @param value The input value to be checked.
 * @param checks A vector of check functions, each taking an input value of type T and returning a bool.
 * @return true If the input value fails any of the provided checks.
 * @return false If the input value passes all the provided checks.
 */
template <typename T> static bool valueFailsChecks(const T value, const std::vector<std::function<bool(T)>> &checks) {
    return std::any_of(checks.begin(), checks.end(), [value](const auto &check) { return check(value); });
}

/**
 * @brief This function performs checks on the input and updates it if necessary.
 *
 * The function first synchronizes any differences between the JSON object and the variables_map object.
 * Then, it enters a loop where it prompts the user to enter a value for the specified key until a valid value is
 * entered. A value is considered valid if it passes all the checks specified in the checkList. If the entered value is
 * invalid, the function catches the exception and prompts the user to enter a new value.
 *
 * @tparam T The type of the value to be checked and updated.
 *
 * @param key The key for which the value is to be checked and updated.
 * @param inputMap The JSON object that contains the current values.
 * @param map The variables_map object that contains the current values.
 * @param checkList A list of functions that perform checks on the value. Each function takes a value of type T and
 * returns a boolean indicating whether the value passed the check.
 *
 * @note The function uses the syncMapKeys function to synchronize the JSON and variables_map objects, and the
 * valueFailsChecks function to check whether a value fails any of the checks.
 * @note The function uses the asNumber function to convert the user's input to a number, and the replace function to
 * update the value in the variables_map object.
 * @note The function catches any exceptions thrown by the asNumber or replace functions and continues prompting the
 * user for a new value.
 */
template <typename T>
static void performChecksAndUpdateInput(std::string key, nlohmann::json &inputMap,
                                        boost::program_options::variables_map &map,
                                        std::vector<std::function<bool(T)>> &checkList) {

    // first, sync any differences between the JSON and variables_map
    syncMapKeys<T>(key, inputMap, map);

    while (map[key].empty() || valueFailsChecks<T>(map[key].as<T>(), checkList)) {
        std::cout << "Enter a value for " << key << ":";
        std::string input;
        std::cin >> input;
        try {
            replace(map, key, static_cast<T>(asNumber(input)));
        } catch (const std::exception &) {
            continue;
        }
    }
}

/**
 * @brief This function prompts the user for input and sets flags in a boost::program_options::variables_map based on
 * the user's input.
 *
 * @param key The key in the variables_map to set. This is also used to check if the flag has already been set.
 * @param description A description of the flag. This is used in the prompt to the user.
 * @param map A reference to the variables_map where the flag will be set.
 *
 * The function first checks if the flag identified by the key has already been set in the variables_map. If it has,
 * the function replaces the value of the flag with the result of the asYesOrNo function called with "yes" as an
 * argument, and then returns.
 *
 * If the flag has not been set, the function enters a loop where it prompts the user with the question "Would you like
 * to use the [description]? [YES/no]: ". The user's input is then read into a string.
 *
 * The function then attempts to replace the value of the flag in the variables_map with the result of the asYesOrNo
 * function called with the user's input as an argument. If this is successful, the flagSet variable is set to true and
 * the loop ends.
 *
 * If an exception is thrown during this process, the function catches it and continues with the next iteration of the
 * loop, prompting the user again.
 */
static void promptAndSetFlags(std::string key, std::string description, boost::program_options::variables_map &map) {
    bool flagSet = map.count(key);
    if (flagSet) {
        replace(map, key, asYesOrNo("yes"));
        return;
    }

    while (!flagSet) {
        try {
            std::cout << "Would you like to use the " << description << "? [YES/no]: ";
            std::string input;
            std::cin >> input;
            replace(map, key, asYesOrNo(input));
            flagSet = true;
        } catch (const std::exception &) {
            continue;
        }
    }
}

}