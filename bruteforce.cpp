#include <iostream>
#include <vector>
#include "tree.h"
#include "pushRelabel.cpp"
#include "printTree.cpp"
#include "readDatasets.cpp"

using namespace std;
#include 
vector<tree*> bruteForce(int depth, vector<int> features){
    vector<tree*> family = {};
    family.push_back(nullptr);
    if(depth == 0){
        return family;
    }
    
    
    for(int i: features){
        features.erase(i);
        vector<tree*> left = bruteForce(new tree(), depth-1, features);
        vector<tree*> right = bruteForce(new tree(), depth-1, features);
        for(tree* t: left){
            for(tree* t2: right){
                tree* tree = new tree();
                tree->featureSplit = i;
                tree->leftChild = t;
                tree->rightChild = t2;
                family.push_back(tree);
            }
        }
    }
    return family;
}

int main(){
    vector<tree*> family = bruteForce(3, features);
    int lowest = INF;
    tree* bestTree = nullptr;
    for(tree* t: family){
        int next = evaluate(t);
        if(lowest > next){
            bestTree = t;
            lowest = next;
        }
    }
    printTree(bestTree, lowest);
}