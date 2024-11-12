#pragma once

/**
 * @file entrypoint.h
 * @author Arjun Earthperson
 * @date 08/30/2023
 * @brief This file contains the definition of the Entrypoint class template.
 *        The Entrypoint class is a generic class that represents a CLI Entrypoint that can be executed.
 */

#include <boost/program_options.hpp>

#include "cmdline.h"

namespace canopy::cli {
/**
 * @class Entrypoint
 * @brief A generic class that represents a CLI Entrypoint that can be executed.
 * @tparam InputType The type of the input data.
 * @tparam CommandLineParserType The type of the command line parser.
 * @tparam OutputType The type of the output data.
 */
template <typename InputType, typename CommandLineParserType, typename OutputType> class Entrypoint {

public:
    /**
     * @brief A static assertion to ensure that CommandLineParserType is a derived class of CommandLine.
     */
    static_assert(std::is_base_of<CommandLine<InputType>, CommandLineParserType>::value,
                  "CommandLineParserType must be a derived class of CommandLine");

    /**
     * @brief Constructor that initializes the command line arguments.
     * @param args The command line arguments.
     */
    explicit Entrypoint(CommandLineArgs args) { cmdArgs = args; }

    /**
     * @brief Default virtual destructor.
     */
    virtual ~Entrypoint() = default;

    virtual /**
     * @brief Executes the CLI Entrypoint.
     */
    void execute() {
        initialize();
        timedRun();
        finalize();
    }

    virtual /**
     * @brief Returns the terminal.
     * @return The terminal.
     */
    CommandLineParserType getTerminal() const { return terminal; }

    /**
     * @brief Returns the input data.
     * @return A reference to the input data.
     */
    InputType &getInputs() {
        return this->inputs;
    }

protected:
    /**
     * @brief Holds the command line arguments.
     */
    CommandLineArgs cmdArgs{};

    /**
     * @brief An instance of CommandLineParserType which is used to parse command line arguments.
     */
    CommandLineParserType terminal;

    /**
     * @brief Holds the output data of the CLI Entrypoint.
     */
    OutputType _outputs;

    virtual /**
     * @brief Initializes the CLI Entrypoint.
     */
    void initialize() {
        terminal = CommandLineParserType(buildHeaderInfo(), cmdArgs);
        terminal.getInputs();
        _outputs = OutputType();
    }

    /**
     * @brief Finalizes the CLI Entrypoint.
     * This is a virtual function and can be overridden by derived classes to provide specific finalization steps.
     */
    virtual void finalize() {

    }

    virtual /**
     * @brief Executes the CLI Entrypoint in a timed manner.
     * This method calls preRun, run, and postRun methods in sequence.
     */
    void timedRun() {
        preRun(_outputs, terminal.getInputs(), terminal.getArguments());

        run(_outputs, terminal.getInputs(), terminal.getArguments());

        postRun(_outputs, terminal.getInputs(), terminal.getArguments());
    }

protected:
    /**
     * @brief Builds the header information.
     * @return The header information.
     */
    virtual HeaderInfo buildHeaderInfo() = 0;

    /**
     * @brief A hook for performing actions before the run.
     * @param output The output data.
     * @param input The input data.
     * @param values The variable map.
     */
    virtual void preRun(OutputType &output, InputType &input, boost::program_options::variables_map &values) {}

    /**
     * @brief Executes the CLI Entrypoint.
     * @param output The output data.
     * @param input The input data.
     * @param values The variable map.
     */
    virtual void run(OutputType &output, InputType &input, boost::program_options::variables_map &values) {}

    /**
     * @brief A hook for performing actions after the run.
     * @param output The output data.
     * @param input The input data.
     * @param values The variable map.
     */
    virtual void postRun(OutputType &output, InputType &input, boost::program_options::variables_map &values) {}

};
}