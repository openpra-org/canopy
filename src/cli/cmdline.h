#pragma once
/**
 * @file cmdline.h
 * @brief This file contains the definition of the CommandLine class and the HeaderInfo struct.
 * @author Arjun Earthperson
 * @date 08/30/2023
 * The CommandLine class is used to parse command line arguments and display information about the CLI entrypoint.
 * The HeaderInfo struct is used to store information about the CLI entrypoint, such as its name and description.
 */

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <utility>

#include "utils/helpers.h"

namespace canopy::cli {

/**
 * @struct HeaderInfo
 * @brief A struct to store information about the CLI entrypoint.
 *
 * This struct contains three members.
 */
typedef struct {
    std::string name;
    std::string description;
    std::string art;
} HeaderInfo;

/**
 * @struct CommandLineArgs
 * @brief A struct to store command line arguments.
 *
 * This struct contains two members: argc and argv. argc is an integer that represents the number of command line
 * arguments, and argv is a pointer to an array of character pointers that represent the command line arguments
 * themselves.
 */
struct CommandLineArgs {
    int argc{};    ///< The number of command line arguments.
    char **argv{}; ///< The array of command line arguments.
};

/**
 * @var default_precision
 * @brief The default precision for output streams.
 *
 * This variable stores the default precision for output streams. It is initialized with the current precision of
 * std::cout.
 */
const auto default_precision{std::cout.precision()};

/**
 * @var max_precision
 * @brief The maximum precision for long double type.
 *
 * This variable stores the maximum precision for long double type. It is initialized with the maximum number of decimal
 * digits that can be represented without change for the long double type.
 */
constexpr auto max_precision{std::numeric_limits<long double>::digits10 + 1};

/**
 * @class CommandLine
 * @brief A class to parse command line arguments and display information about the CLI entrypoint.
 *
 * This class contains methods to parse command line arguments using the Boost library, and to display information about
 * the CLI entrypoint. The constructor takes a HeaderInfo object and command line arguments, and initializes the class.
 * getArguments method returns the parsed command line arguments. The printLine method prints a line to the console. The
 * buildGenerics method builds a set of generic command line options. The printHeader method prints the header
 * to the console. The printPrecisionInformation method prints information about the precision of long double values.
 */

template <typename InputType> class CommandLine {

  public:
    /**
     * @brief Constructor for the CommandLine class.
     * @param headerInfo A HeaderInfo object containing information about the CLI entrypoint.
     * @param args Command line arguments.
     * This constructor initializes the CommandLine object with the provided HeaderInfo object and command line
     * arguments. It also prints the entrypoint header.
     */
    explicit CommandLine(HeaderInfo headerInfo, CommandLineArgs args) {
        cmdArgs = args;
        initialized = false;
        header = std::move(headerInfo);
    }

    /**
     * @brief Default constructor for the CommandLine class.
     * This constructor initializes the CommandLine object with an empty variables map.
     */
    explicit CommandLine() { variablesMap = boost::program_options::variables_map(); }

    /**
     * @brief Destructor for the CommandLine class.
     * This is a virtual destructor that does nothing.
     */
    virtual ~CommandLine() = default;

    /**
     * @brief Method to get the parsed command line arguments.
     * @return A reference to the variables map that contains the parsed command line arguments.
     * This method returns the parsed command line arguments. If the CommandLine object is not initialized,
     * it calls the initialize method before returning the arguments.
     */
    boost::program_options::variables_map &getArguments() {
        if (!initialized) {
            initialize();
        }
        return variablesMap;
    }

    /**
     * @brief Method to get a copy of the parsed command line arguments.
     * @return A copy of the variables map that contains the parsed command line arguments.
     * This method returns a copy of the parsed command line arguments. If the CommandLine object is not initialized,
     * it calls the initialize method before returning the arguments.
     */
    boost::program_options::variables_map getArgumentsMap() {
        if (!initialized) {
            initialize();
        }
        return variablesMap;
    }

    /**
     * @brief Method to get the command line argv, argc objects.
     * @return A const reference to the CommandLineArgs object that contains the command line arguments.
     * This method returns the command line arguments that were passed to the program.
     */
    [[nodiscard]] const CommandLineArgs &getCmdArgs() const { return cmdArgs; }

    /**
     * @brief Method to get the current precision.
     * @return The current precision as an integer.
     * This method returns the current precision of long double values.
     */
    int getCurrentPrecision() { return variablesMap["precision"].as<int>(); }

    /**
     * @brief Method to set the precision of long double values.
     * @param precisionToSet The desired precision as an integer (default value is 5).
     * This method sets the precision of long double values to the specified value.
     * If no value is provided, the default precision is set to 5.
     */
    void setPrecision(int precisionToSet = 5) {
        utils::replace(variablesMap, "precision", std::setprecision(precisionToSet)._M_n);
    }

    /**
     * @brief Method to get the inputs.
     * @return A reference to the inputs.
     * This method returns the inputs. If the CommandLine object is not initialized,
     * it calls the initialize method before returning the inputs.
     */
    InputType &getInputs() {
        if (!initialized) {
            initialize();
        }
        return _inputs;
    }

    /**
     * @brief Method to print a line to the console.
     * This method prints a line of dashes to the console.
     */
    static void printLine() {
        std::cout << R"(--------------------------------------------------------------------------------)"
                  << "\n";
    }

  protected:
    /**
     * @brief Method to build a set of input arguments.
     * @param inputArguments A reference to an options_description object that will be filled with the input arguments.
     * This is a pure virtual method that must be implemented by derived classes. It is used to build a set of input
     * arguments for the command line parser.
     */
    virtual void buildInputArguments(boost::program_options::options_description &inputArguments) = 0;

    /**
     * @brief Method to print the input arguments.
     * @param values A reference to a variables_map object that contains the parsed command line arguments.
     * This is a pure virtual method that must be implemented by derived classes. It is used to print the input
     * arguments to the console.
     */
    virtual void printInputArguments(boost::program_options::variables_map &values) = 0;

    /**
     * @brief Method to perform a check on the input arguments.
     * @param values A reference to a variables_map object that contains the parsed command line arguments.
     * This is a pure virtual method that must be implemented by derived classes. It is used to perform a check on the
     * input arguments to ensure they are valid.
     */
    virtual void performInputArgumentsCheck(boost::program_options::variables_map &values) = 0;

    /**
     * @brief Method to fill the inputs object based on the parsed command line arguments.
     * @param ToFill A reference to the inputs object that will be filled.
     * @param values A reference to a variables_map object that contains the parsed command line arguments.
     * This is a pure virtual method that must be implemented by derived classes. It is used to fill the inputs object
     * based on the parsed command line arguments.
     */
    virtual void buildInputs(InputType &ToFill, boost::program_options::variables_map &values) = 0;

    /**
     * @var variablesMap
     * @brief A variables_map object to store the parsed command line arguments.
     *
     * This object is used to store the parsed command line arguments. It is filled by the Boost library's command line
     * parser.
     */
    boost::program_options::variables_map variablesMap;

    /**
     * @var initialized
     * @brief A boolean flag to indicate whether the CommandLine object has been initialized.
     *
     * This flag is set to true when the CommandLine object is initialized, and is used to prevent the object from being
     * initialized more than once.
     */
    bool initialized = false;

    /**
     * @var cmdArgs
     * @brief A CommandLineArgs object to store the command line arguments.
     *
     * This object is used to store the command line arguments that were passed to the program.
     */
    CommandLineArgs cmdArgs;

    /**
     * @var inputs
     * @brief An InputType object to store the inputs.
     *
     * This object is used to store the inputs that are parsed from the command line arguments.
     */
    InputType _inputs = InputType();

    /**
     * @var header
     * @brief A HeaderInfo object to store information about the CLI entrypoint.
     *
     * This object is used to store information about the CLI entrypoint, such as the CLI entrypoint name, CLI
     * entrypoint description, submission date, and student name.
     */
    HeaderInfo header;

    /**
     * @brief Method to build a set of generic command line options.
     * @return An options description containing the generic command line options.
     * This method builds a set of generic command line options, such as "help", "quiet", and "precision",
     * and returns them as an options description.
     */
    static boost::program_options::options_description buildGenerics() {
        boost::program_options::options_description generics("General options");
        generics.add_options()("help,h", "= Show this help message")("quiet,q", "= Reduce verbosity")(
            "precision,p", boost::program_options::value<int>()->default_value(15),
            "= Number of digits to represent long double");
        return generics;
    }

    /**
     * @brief Method to print the CLI entrypoint header to the console.
     * @param headerInfo A reference to a HeaderInfo object containing information about the CLI entrypoint.
     * This method prints the CLI entrypoint name, CLI entrypoint description to the console.
     */
    static void printHeader(const HeaderInfo &headerInfo) {
        if (!headerInfo.art.empty()) {
            std::cout << headerInfo.art << std::endl;
        }
        printLine();
    }

    /**
     * @brief Method to print information about the precision of long double values.
     * This method prints the default, maximum, and current precision of long double values to the console.
     */
    void printPrecisionInformation() {
        const auto precision = variablesMap["precision"].as<int>();
        std::cout << std::setprecision(static_cast<int>(precision));
        std::cout << "\t\t\tPrecision in digits:  ";
        std::cout << "default: " << default_precision << ", ";
        std::cout << "maximum: " << max_precision << ", ";
        std::cout << "current: " << precision << "\n";
        printLine();
    }

    virtual /**
             * @brief Method to initialize the CommandLine object.
             * This method initializes the CommandLine object by printing the compile configurations and precision
             * information, and parsing the command line arguments. If the CommandLine object is already initialized, it
             * does nothing.
             */
        void
        initialize() {

        if (initialized) {
            return;
        }

        initialized = true;
        boost::program_options::options_description options("Parameters");
        buildInputArguments(options);
        options.add(buildGenerics());
        boost::program_options::store(boost::program_options::parse_command_line(cmdArgs.argc, cmdArgs.argv, options),
                                      variablesMap);
        boost::program_options::notify(variablesMap);

        // if help, print options and exit
        if (variablesMap.contains("help")) {
            if (!variablesMap.contains("quiet")) {
                printHeader(header);
            }
            std::cout << options << "\n";
            exit(0);
        }

        // print compiler information
        if (!variablesMap.count("quiet")) {
            // print command line options
            std::cout << options << "\n";
            printLine();
            printPrecisionInformation();
        }

        // consume and correct the input arguments
        performInputArgumentsCheck(variablesMap);

        // if (variablesMap.count("bench")) {
        //     // build the profiler now
        //     ProfilerHelper::validateOptions(variablesMap);
        // }

        if (!variablesMap.count("quiet")) {
            // print the input arguments
            printInputArguments(variablesMap);
        }

        // finally, build and save the _inputs object
        buildInputs(_inputs, variablesMap);
    }
};
} // namespace canopy::cli
