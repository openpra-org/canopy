#include "cli/eval/argparse.h"

#include "cli/cmdline.h"
#include "cli/entrypoint.h"

#include "utils/helpers.h"

namespace canopy::cli {

class Eval : public Entrypoint<EvalInput, EvalParser, EvalOutput> {
  public:
    /**
     * @brief Constructor for the cli
     * @param args Command line arguments
     */
    explicit Eval(CommandLineArgs args) : Entrypoint(args) {}

  protected:
    /**
     * @brief This function builds the header information for this cli
     * @return HeaderInfo object containing project information
     */
    HeaderInfo buildHeaderInfo() override {

        utils::Canvas canvas = {
            .c = std::complex(-0.292, -0.66),
            .x_bounds = {-0.007514104707, 0.075446744304},
            .y_bounds = {0.825578589953, 0.883651184261},
        };
        utils::FractalArt art = {
            .canvas = canvas,
            .max_iterations = 200,
        };
        return {
            .art = canopy::utils::drawJuliaSet(art),
        };
    }

    /**
     * @brief This function prints the results of the computation.
     * @param results The results of the computation.
     */
    static void printResults(EvalOutput &results) {
    }

    void run(EvalOutput &outputs, EvalInput &inputs, boost::program_options::variables_map &values) override {

        nlohmann::json results;
        //inputs.toJSON(results["inputs"]);

        // if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_POINT_JACOBI)) {
        //     InLab6Outputs pointJacobiResults(inputs);
        //     Compute::usingPointJacobi(pointJacobiResults, inputs);
        //     pointJacobiResults.toJSON(
        //         results["outputs"][MyRelaxationMethod::TypeKey(MyRelaxationMethod::Type::METHOD_POINT_JACOBI)]);
        //     Parser::printLine();
        //     std::cout << "Point Seidel Method Results" << std::endl;
        //     Parser::printLine();
        //     printResults(pointJacobiResults);
        // }
        //
        // if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_GAUSS_SEIDEL)) {
        //     Parser::printLine();
        //     std::cout << "Gauss Seidel not implemented yet." << std::endl;
        // }
        //
        // if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_SOR)) {
        //     Parser::printLine();
        //     std::cout << "Successive over-relaxation (SOR) not implemented yet." << std::endl;
        // }
        //
        // if (inputs.methods.count(MyRelaxationMethod::Type::METHOD_SSOR)) {
        //     Parser::printLine();
        //     std::cout << "Symmetric Successive over-relaxation (SOR) not implemented yet." << std::endl;
        // }
        //
        // Parser::printLine();
        // writeJSON(values["output-json"].as<std::string>(), results);
    }
};
}
/**
 * @brief The main function of the program.
 *
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 *
 * @return The exit status of the program.
 */
int main(const int argc, char **argv) {
    const canopy::cli::CommandLineArgs args = {
        .argc = argc,
        .argv = argv,
    };
    auto entrypoint = canopy::cli::Eval(args);
    entrypoint.execute();
}
