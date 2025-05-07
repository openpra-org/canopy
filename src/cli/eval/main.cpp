#include "cli/eval/argparse.h"

#include "cli/cmdline.h"
#include "cli/entrypoint.h"

#include "utils/helpers.h"

namespace canopy::cli {

class Eval : public Entrypoint<EvalInput, EvalParser, EvalOutput> {
  public:
    /**
     * @brief Constructor for the outlab5 class
     * @param args Command line arguments
     */
    explicit Eval(CommandLineArgs args) : Entrypoint(args) {}

  protected:
    /**
     * @brief This function builds the header information for the project.
     * @return HeaderInfo object containing project information
     */
    HeaderInfo buildHeaderInfo() override {
        canopy::utils::Canvas canvas;
        auto x = -0.292;
        auto y = -0.66;
        size_t iterations = 200;
        canvas.x_start = -0.007514104707;
        canvas.x_stop = 0.075446744304;
        canvas.y_start = 0.825578589953;
        canvas.y_stop = 0.883651184261;
        canvas.tone_map.growth_rate = 0.3;
        canopy::utils::printJuliaSet<__float128>(canvas, x, y, iterations); //"o█■"
        return {
            .name = "\n",
            .description = "\n",
        };
    }

    /**
     * @brief This function prints the results of the computation.
     * @param results The results of the computation.
     */
    static void printResults(EvalOutput &results) {
    }

    /**
     * @brief This function runs the project.
     * @details It solves the system of linear equations using forward and back substitution.
     * @param outputs The output vector
     * @param inputs The input matrices
     * @param values The variable map
     */
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
int main(int argc, char **argv) {
    canopy::cli::CommandLineArgs args = {
        .argc = argc,
        .argv = argv,
    };
    auto entrypoint = canopy::cli::Eval(args);
    entrypoint.execute();
}
