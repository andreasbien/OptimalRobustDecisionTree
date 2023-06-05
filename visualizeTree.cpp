#include "tree.h"

#include <iostream>
#include <string>
#include <deque>
#include <cmath>
#include <iomanip>

void printTree(tree* root, int totalSamples) {
    if(!root) return;

    int levels = 0;
    std::deque<tree*> queue;
    queue.push_back(root);

    // calculate the height of the tree
    while(!queue.empty()) {
        ++levels;
        std::deque<tree*> nextLevel;

        while(!queue.empty()) {
            tree* current = queue.front();
            queue.pop_front();

            if(current->leftChild)
                nextLevel.push_back(current->leftChild);

            if(current->rightChild)
                nextLevel.push_back(current->rightChild);
        }

        queue = nextLevel;
    }

    // print the tree level by level
    queue.push_back(root);
    int level = 0;

    while(!queue.empty()) {
        std::deque<tree*> nextLevel;
        int paddingWidth = std::pow(2, levels - level) - 1;
        std::string nodePadding(paddingWidth, ' ');
        std::string branchPadding(paddingWidth - 1, ' ');

        // Save nodes' information for printing branches
        std::vector<std::string> nodesInfo;
        std::vector<tree*> nodes;

        while(!queue.empty()) {
            tree* current = queue.front();
            queue.pop_front();

            std::string nodeValue = (current->leftChild == nullptr && current->rightChild == nullptr) ?
                                     "(" + std::to_string(current->instances_true) + "," + std::to_string(current->instances_false) + ")" :
                                     std::to_string(current->featureSplit);

            std::cerr << nodePadding << nodeValue << nodePadding;
            nodesInfo.push_back(nodeValue);
            nodes.push_back(current);

            if(current->leftChild)
                nextLevel.push_back(current->leftChild);

            if(current->rightChild)
                nextLevel.push_back(current->rightChild);
        }

        queue = nextLevel;
        ++level;
        std::cerr << std::endl;

        // Print branches
        if (!queue.empty()) {
            for (size_t i = 0; i < nodes.size(); ++i) {
                std::cerr << branchPadding;
                std::cerr << (nodes[i]->leftChild ? "/" : " ");
                std::cerr << std::setw(nodesInfo[i].size()) << std::setfill(' ') << "";
                std::cerr << (nodes[i]->rightChild ? "\\" : " ");
                std::cerr << branchPadding;
            }
            std::cerr << std::endl;
        }
    }
    std::cerr << totalSamples << std::endl;
}
