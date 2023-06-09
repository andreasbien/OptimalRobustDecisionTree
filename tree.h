#ifndef TREE_H
#define TREE_H

#include <vector>

struct tree {
    int featureSplit;
    int lowerBound = 1000000;
    int upperBound = 1000000;
    int local_lowerBound = 1000000;
    tree* leftChild = nullptr;
    tree* rightChild = nullptr;
    std::vector<tree*> leaf_nodes = {};
    int instances_true = 1000000;
    int instances_false = 1000000;
};

#endif
