#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "instance_t.h"

using namespace std;



vector<instance_t*> readInstances(const string& filename) {
    vector<instance_t*> instances;

    // Open the file
    ifstream file(filename);
    if (!file) {
        cerr << "Could not open the file: " << filename << endl;
        return instances;  // Return empty vector
    }

    string line;
    while (getline(file, line)) {
        // Create a new instance
        instance_t* instance = new instance_t();

        // Parse the line
        istringstream iss(line);

        // Read label
        iss >> instance->label;

        // Read features
        int feature;
        while (iss >> feature) {
            instance->features.push_back(feature);
        }

        // Add the instance to the vector
        instances.push_back(instance);
    }

    return instances;
}

// int main() {
//     const string filename = "input.in";  // Replace with your .in file
//     vector<instance_t*> instances = readInstances(filename);

//     // Print out the instances
//     for (const auto& instance : instances) {
//         cout << "Label: " << instance->label << ", Features: ";
//         for (const auto& feature : instance->features) {
//             cout << feature << ' ';
//         }
//         cout << '\n';
//         delete instance; // delete the instance once it's processed to avoid memory leak
//     }

//     return 0;
// }
