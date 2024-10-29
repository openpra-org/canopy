#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

// Function to multiply two matrices
vector<vector<double>> multiplyMatrices(const vector<vector<double>>& A, const vector<vector<double>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size(); // Assuming A is not empty and is a valid matrix
    int rowsB = B.size();
    int colsB = B[0].size();

    // Check if multiplication is possible
    if (colsA != rowsB) {
        cerr << "Matrix dimensions incompatible for multiplication." << endl;
        exit(1);
    }

    // Initialize result matrix with zeros
    vector<vector<double>> result(rowsA, vector<double>(colsB, 0.0));

    // Perform multiplication
    for (int i = 0; i < rowsA; ++i) {
        for (int k = 0; k < colsA; ++k) {
            for (int j = 0; j < colsB; ++j) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

// Function to print a matrix
void printMatrix(const vector<vector<double>>& matrix, const string& name) {
    cout << name << " = " << endl;
    for (const auto& row : matrix) {
        cout << "[ ";
        for (const auto& val : row) {
            cout << fixed << setprecision(5) << setw(9) << val << " ";
        }
        cout << "]" << endl;
    }
    cout << endl;
}

int main() {
    // Matrix M1: Probabilities from entry states to exit states (miss or hit)
    // Entry states: Normal, Sleepy, Speeding
    // Exit states: Miss, Hit
    vector<vector<double>> M1 = {
            {0.84, 0.16}, // Normal driver
            {0.60, 0.40}, // Sleepy driver
            {0.66, 0.34}  // Speeding driver
    };

    // Matrix M2: Probabilities from exit states to consequences
    // Entry states: Miss, Hit
    // Exit states: Consequence states 1 to 5
    vector<vector<double>> M2 = {
            {1.0, 0.0, 0.0, 0.0, 0.0}, // Miss
            {0.0, 0.2, 0.6, 0.1, 0.1}  // Hit
    };

    // Multiply M1 and M2 to get M3
    vector<vector<double>> M3 = multiplyMatrices(M1, M2);

    // Output the matrices
    printMatrix(M1, "M1");
    printMatrix(M2, "M2");
    printMatrix(M3, "M3");

    // Interpret the results
    vector<string> entryStates = {"Normal", "Sleepy", "Speeding"};
    vector<string> consequenceStates = {
            "Alive, None, Not Cancelled",
            "Dead, Minor, Not Cancelled",
            "Dead, Moderate, Not Cancelled",
            "Dead, Major, Not Cancelled",
            "Dead, Major, Cancelled"
    };

    for (size_t i = 0; i < M3.size(); ++i) {
        cout << "Entry State: " << entryStates[i] << endl;
        for (size_t j = 0; j < M3[i].size(); ++j) {
            cout << "  Probability of '" << consequenceStates[j] << "': "
                 << fixed << setprecision(5) << M3[i][j] << endl;
        }
        cout << endl;
    }

    return 0;
}