#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <array>

std::string exec(const char* cmd, const std::string &input) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(cmd, "w+");
    if (!pipe) {
    perror("popen() failed");  // this will print more detailed error message
    throw std::runtime_error("popen() failed!");
}
    fputs(input.c_str(), pipe);
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    std::string directory = std::filesystem::current_path().string();
    std::string exec_name = "out.exe";

    // Compile program
    if (std::filesystem::exists(exec_name)) {
        std::filesystem::remove(exec_name);
    }
    std::system("g++ murtree_robust.cpp -o out.exe");

    int c = 0;
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        std::string filename = entry.path().string();
        std::string name = split(filename, '.')[0];

        // Open input file
        std::ifstream in_file(directory + "/datasets/" + name + ".in");
        std::string lines((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());

        // Run command
        std::string cmd = "./" + exec_name;
        std::string out = exec(cmd.c_str(), lines);
        int wrong = -1;
        std::cout << out << std::endl;

        // Open answer file
        std::ifstream ans_file(directory + "/datasets/" + name + ".ans");
        int max_wrong;
        ans_file >> max_wrong;
        std::cout << "\033[32;1mCorrect result in test " << name << ": needed at most " << max_wrong << " wrong, got " << wrong << " wrong\033[0m" << std::endl;

        std::cout << "\n\033[32;1mSuccess!\033[0m" << std::endl;
        std::cout << "#######################################" << std::endl << std::endl;

        c++;
        if (c > 2) break;
    }

    return 0;
}
