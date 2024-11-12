#pragma once

#include "args.h"
#include "cli/cmdline.h"
#include "io/parser.h"
#include "utils/checks.h"
#include "utils/helpers.h"

namespace canopy::cli {

class EvalParser : public CommandLine<EvalInput> {

  public:
    explicit EvalParser(const HeaderInfo &headerInfo, const CommandLineArgs &args) : CommandLine(headerInfo, args) {}

    explicit EvalParser() = default;

  protected:
    /**
     * @brief This function builds the input options for the program.
     *
     * @return A boost::program_options::options_description object containing the description of the input options.
     */
    void buildInputArguments(boost::program_options::options_description &values) override {
        values.add_options()("threshold,t", boost::program_options::value<long double>(),
                             "= iterative convergence threshold [𝜀 > 0]")(
            "max-iterations,k", boost::program_options::value<long double>(), "= maximum number of iterations [n ∈ ℕ]")(
            "order,n", boost::program_options::value<long double>(), "= order of the square matrix [n ∈ ℕ]")(
            "input-json,i", boost::program_options::value<std::string>(), "= input JSON containing A, and b")(
            "output-json,o", boost::program_options::value<std::string>(), "= path for the output JSON");

        boost::program_options::options_description methods("Solver Methods");
        methods.add_options()("use-point-jacobi", "= Use the Point-Jacobi method")(
            "use-gauss-seidel", "= [DISABLED] Use the Gauss-Seidel method")(
            "use-SOR", "= [DISABLED] Use the SOR method")("use-SSOR", "= [DISABLED] Use the symmetric SOR method");
        values.add(methods);
    }

    /**
     * @brief This function prints the input values.
     *
     * @param vm A boost::program_options::variables_map object containing the input values to be printed.
     *
     */
    void printInputArguments(boost::program_options::variables_map &vm) override {
        // list the parameters
        CommandLine::printLine();
        std::cout << std::setw(44) << "Inputs\n";
        CommandLine::printLine();
        std::cout << "\tInput JSON (for A, b),  i: " << vm["input-json"].as<std::string>() << "\n";
        std::cout << "\tOutput JSON (for x),    o: " << vm["output-json"].as<std::string>() << "\n";
        std::cout << "\tConvergence Threshold,  𝜀: " << vm["threshold"].as<long double>() << "\n";
        std::cout << "\tMax iterations,         k: " << static_cast<size_t>(vm["max-iterations"].as<long double>())
                  << "\n";
        std::cout << "\tMatrix order,           n: "
                  << (vm.count("order") ? std::to_string(static_cast<size_t>(vm["order"].as<long double>()))
                                        : "None provided, will be inferred from input JSON")
                  << "\n";
        std::cout << "\tUse Gauss-Siedel         : " << (vm["use-gauss-seidel"].as<bool>() ? "Yes" : "No") << "\n";
        std::cout << "\tUse Point-Jacobi         : " << (vm["use-point-jacobi"].as<bool>() ? "Yes" : "No") << "\n";
        std::cout << "\tUse SOR                  : " << (vm["use-SOR"].as<bool>() ? "Yes" : "No") << "\n";
        std::cout << "\tUse SSOR                 : " << (vm["use-SSOR"].as<bool>() ? "Yes" : "No") << "\n";
        CommandLine::printLine();
    }

    /**
     * @brief This function performs checks on the input parameters and prompts the user to enter valid inputs if the
     * current inputs are invalid.
     *
     * @param map A boost::program_options::variables_map object containing the input values to be checked.
     */
    /**
     * @brief This function performs checks on the input parameters and prompts the user to enter valid inputs if the
     * current inputs are invalid.
     *
     * @param map A boost::program_options::variables_map object containing the input values to be checked.
     */
    void performInputArgumentsCheck(boost::program_options::variables_map &map) override {

        std::string input;

        // Check if input file path is provided
        if (!map.count("input-json") || map["input-json"].empty() ||
            !canopy::io::doesFileExist(map["input-json"].as<std::string>())) {
            while (!map.count("input-json") || map["input-json"].empty() ||
                   !io::doesFileExist(map["input-json"].as<std::string>())) {
                std::cerr << "Error: No input JSON filepath provided.\n" << std::endl;
                std::cout << "Enter input file path (file extension is .json): ";
                std::cin >> input;
                try {
                    utils::replace(map, "input-json", input);
                } catch (const std::exception &) {
                    continue;
                }
            }
        }

        // Check if output file path is provided and writable
        if (!map.count("output-json") || map["output-json"].empty() ||
            !io::isFileWritable(map["output-json"].as<std::string>())) {
            while (!map.count("output-json") || map["output-json"].empty() ||
                   !io::isFileWritable(map["output-json"].as<std::string>())) {
                std::cerr << "Error: No output JSON filepath provided.\n" << std::endl;
                std::cout << "Enter output file path (file extension is .json): ";
                std::cin >> input;
                try {
                    utils::replace(map, "output-json", input);
                } catch (const std::exception &) {
                    continue;
                }
            }
        }

        // read the input json and populate the variables_map
        nlohmann::json inputMap;
        try {
            io::readJSON(map["input-json"].as<std::string>(), inputMap);
        } catch (...) {
            // initialize input map if no file was read
        }

        std::vector<std::function<bool(long double)>> checks;

        // add checks for parameters 𝜀 and k
        checks.clear();
        checks.emplace_back([](long double value) { return utils::failsPositiveNumberCheck(value); });
        utils::performChecksAndUpdateInput<long double>("threshold", inputMap, map, checks);

        checks.emplace_back([](long double value) { return utils::failsWholeNumberCheck(value); });
        utils::performChecksAndUpdateInput<long double>("max-iterations", inputMap, map, checks);

        if (map.count("order")) {
            utils::performChecksAndUpdateInput<long double>("order", inputMap, map, checks);
        }

        utils::promptAndSetFlags("use-point-jacobi", "Point Jacobi method", map);
        utils::promptAndSetFlags("use-gauss-seidel", "Gauss-Seidel method", map);
        utils::promptAndSetFlags("use-SOR", "SOR method", map);
        utils::promptAndSetFlags("use-SSOR", "symmetric SOR method", map);
    }

    /**
     * @brief Builds the input matrices for the MyBLAS library from a JSON file.
     *
     * This function reads the input JSON file and extracts the coefficient matrix and constants vector.
     * It also validates the dimensions of the input matrices and sets the order of the system.
     * If the verbose mode is enabled, it prints the input matrices.
     *
     * @param inputs Reference to a MyBLAS::InputMatrices object to store the input matrices.
     * @param values Reference to a boost::program_options::variables_map object containing the command line arguments.
     */
    void buildInputs(EvalInput &inputs, boost::program_options::variables_map &values) override {

        // first, read the input file into a json map
        nlohmann::json inputMap;
        io::readJSON(values["input-json"].as<std::string>(), inputMap);

        // read the constants
        std::vector<long double> constants = std::vector(std::vector<long double>(inputMap["constants"]));
        //inputs.constants = MyBLAS::Vector(constants);

        // read the coefficient matrix
        //inputs.coefficients = MyBLAS::Matrix(std::vector<std::vector<long double>>(inputMap["coefficients"]));

        // if (!MyBLAS::isSquareMatrix(inputs.coefficients)) {
        //     std::cerr << "Error: Input coefficients matrix A not square, aborting.\n";
        //     exit(-1);
        // }

        // if (inputs.coefficients.getRows() != inputs.constants.size()) {
        //     std::cerr << "Error: Input constants vector not order n, aborting.\n";
        //     exit(-1);
        // }

        // set input order n
        // const auto orderFromInputMatrixDimensions = inputs.coefficients.getRows();
        // if (!values.count("order")) {
        //     std::cout << "Reading matrix order (n) from input matrix dimensions: " << orderFromInputMatrixDimensions
        //               << "\n";
        //     inputs.n = orderFromInputMatrixDimensions;
        // } else { // user provided some value for n, validate against input dimensions
        //     const auto orderFromUser = static_cast<size_t>(values["order"].as<long double>());
        //     inputs.n = orderFromUser > orderFromInputMatrixDimensions ? orderFromInputMatrixDimensions : orderFromUser;
        //     if (orderFromUser > orderFromInputMatrixDimensions) {
        //         std::cerr << "Warning: Matrix order (n) is larger than input matrix, defaulting to lower value\n";
        //     }
        // }
        //
        // inputs.threshold = values["threshold"].as<long double>();
        // inputs.max_iterations = static_cast<size_t>(values["max-iterations"].as<long double>());
        //
        // if (values["use-point-jacobi"].as<bool>()) {
        //     inputs.methods.insert(MyRelaxationMethod::Type::METHOD_POINT_JACOBI);
        // }
        //
        // if (values["use-gauss-seidel"].as<bool>()) {
        //     inputs.methods.insert(MyRelaxationMethod::Type::METHOD_GAUSS_SEIDEL);
        // }
        //
        // if (values["use-SOR"].as<bool>()) {
        //     inputs.methods.insert(MyRelaxationMethod::Type::METHOD_SOR);
        // }
        //
        // if (values["use-SSOR"].as<bool>()) {
        //     inputs.methods.insert(MyRelaxationMethod::Type::METHOD_SSOR);
        // }
        //
        // // print the matrices since we are in verbose mode.
        // if (!values.count("quiet")) {
        //     const auto precision = getCurrentPrecision();
        //     printLine();
        //     std::cout << "Coefficient Matrix (A):\n";
        //     printLine();
        //     std::cout << std::setprecision(static_cast<int>(precision)) << inputs.coefficients;
        //     printLine();
        //     std::cout << "Constants Vector (b):\n";
        //     printLine();
        //     std::cout << std::setprecision(static_cast<int>(precision)) << inputs.constants;
        // }
    }
};
}