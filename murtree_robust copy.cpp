/**
 * This is a simplified version of the MurTree algorithm I made for
 * my Honours project. It doesn't contain everything, but it at least
 * contains:
 *  - Caching (DP)
 *  - A specialized algorithm for depth-2 trees
 *
 * Note that this algorithm ONLY WORKS WITH BINARY FEATURES AND
 * LABELS. This is most likely not what our actual datasets are going
 * to look like.
 */

#include <bits/stdc++.h>
#include <map>
#include <stdio.h>
#include <vector>
#include "pushRelable.cpp"
#include "visualizeTree.cpp"
#include "tree.h"
#include "instance_t.h"
#include "readDatasets.cpp"

#define INF INT32_MAX / 2

using namespace std;

bool compareByUpperBound(const int &upperBoundA, const int &upperBoundB){
    return upperBoundA < upperBoundB;
}

struct tree_solutions {
    int lowerBound;
    int upperBound;
    vector<tree*> trees = {};
};


/**
 * This is a preprocessing algorithm that removes features if
 * one of the following conditions applies:
 *  - The feature is always 0 or 1 for a certain instance (and
 *      thus informationless)
 *  - The feature is the exact opposite of the feature that appears
 *      before it (always 0 if the previous feature is 1, and 1 if
 *      the previous feature is 0)
 * This last case in particular occurs a lot in the datasets for
 * which we had to solve the problem, most likely due to choices
 * made in the binarization step.
 */
vector<instance_t*> filter_instects(vector<instance_t*> insts) {
    
    
    int height = insts.size();
    int width = insts[0]->features.size();

    vector<instance_t*> filtered_insts {};
    for (instance_t* inst : insts) {
        instance_t* filtered_inst = new instance_t;
        *filtered_inst = {inst->label, {inst->features[0]}};
        filtered_insts.push_back(filtered_inst);
    }

    for (int ix = 1; ix < width; ix++) {
        int c[2] {0, 0};
        for (instance_t* inst : insts) {
            c[inst->features[ix]]++;
        }
        if (c[0] == 0 || c[1] == 0)
            continue;

        bool complement = true;
        for (instance_t* inst : insts) {
            if (inst->features[ix - 1] == inst->features[ix]) {
                complement = false;
                break;
            }
        }
        if (complement)
            continue;

        for (int iy = 0; iy < height; iy++) {
            filtered_insts[iy]->features.push_back(insts[iy]->features[ix]);
        }
    }

    return filtered_insts;
}

/**
 * The memoization map that will map a certain branch (represented
 * by an integer key) to the amount of misclassifications that we
 * found for it.
 */
map<int, tree_solutions*> mem = {};


/**
 * Simple float-int pair.
 */
struct pair_t {
    float x;
    int y;
};

/**
 * Simple comparison function between pairs, sorts ascendingly on x.
 */
bool compare(pair_t a, pair_t b)
{
    return a.x < b.x;
}

/**
 * Simple min-routine.
 */
int min(int a, int b) {
    return a < b ? a : b;
}

/**
 * The main algorithm that does MurTree-stuff.
 * Given are:
 *  - a list of instects it needs to classify,
 *  - the depth the tree should have (how many times it can still
 *      recurse),
 *  - a "path" of features (integers representing what has already
 *      been filtered on so far, useful for caching),
 *  - an upper bound, useful for deciding whether it's still possible
 *      to improve, and terminating if that's not the case.
 */
void deleteTree(tree* t){
    if(t->leftChild != nullptr)
        deleteTree(t->leftChild);
    if(t->rightChild != nullptr)
        deleteTree(t->rightChild);
    delete t;
    
}
void filter(tree_solutions &candidates, tree_solutions &solution) {
    int upperBound = candidates.upperBound;
    vector<tree*> trees = solution.trees;
    
        
    
    if(upperBound == candidates.lowerBound){
        bool found = false;
        for (auto & t : trees) {
        if (t->upperBound == upperBound) {
            if(!found)
                candidates.trees.push_back(t);
            else
               delete t;
            found = true;
        }
    }
    }

    else{
        for (auto & t : trees) {
        if (t->lowerBound < upperBound) {
            candidates.trees.push_back(t);
        }
        else{
            delete t;
        }
    }
    }
    
}

vector<int> getLeafConnections(instance_t* &inst, tree* t){
    if(t == nullptr)
        return std::vector<int>(1,0);
    if(inst->features[t->featureSplit] == 0)
        return getLeafConnections(inst,t->leftChild);
    else if(inst->features[t->featureSplit] == 1)
        return getLeafConnections(inst,t->rightChild);
    else{
        vector<int> leafsLeft = getLeafConnections(inst,t->leftChild);
        vector<int> leafsRight = getLeafConnections(inst,t->rightChild);
        int leafsInLeft = t->leftChild->leaf_nodes.size();
        std::transform(leafsRight.begin(), leafsRight.end(), leafsRight.begin(),
                   [leafsInLeft](int element) { return element + leafsInLeft; });

        
        std::vector<int> leafsBoth;
        leafsBoth.reserve(leafsLeft.size() + leafsRight.size());  
        leafsBoth.insert(leafsBoth.end(), leafsLeft.begin(), leafsRight.end());

        return leafsBoth;
    }


    

}
tree* combineTrees(const vector<instance_t*> &insts,const vector<instance_t*> &globalInsts, int feature, tree &left, tree &right){
    
    tree* combined = new tree();
    combined->featureSplit = feature;
    combined->leftChild= &left;
    combined->rightChild = &right;
    
    
    
    
    vector<tree*> leftLeafNodes = left.leaf_nodes;
    vector<tree*> rightLeafNodes = right.leaf_nodes;
   
    
    vector<tree*> leafs(leftLeafNodes);
    
    
    leafs.insert(leafs.begin(),rightLeafNodes.begin(), rightLeafNodes.end());
    
    combined->leaf_nodes = leafs;
    
    Graph g(leafs.size()+2);
    int source = 0;
    int currentNode = 1;
    int sink = leafs.size() + 1;

    
    for(tree* leaf: leafs){
        
        
        
         g.addEdge(source,currentNode,leaf->instances_true);
         g.addEdge(currentNode, sink, leaf->instances_false);
        currentNode++;

    }

    int upperBound = g.getMaxFlow(source, sink);
    
    combined->upperBound = upperBound;
    combined->lowerBound = upperBound;
    
    return combined;

}

tree_solutions* merge(const vector<instance_t*> &insts,const vector<instance_t*> &globalInsts, int feature, tree_solutions &left, tree_solutions &right){
    
    vector<tree*> merged = {};

  
    int c = 0;
    int a = 0;
    for (auto & r : right.trees) {
        a++;
        for (auto & l : left.trees) {
            c++;
            tree* combined = combineTrees(insts,globalInsts,feature, *l,*r);
            merged.push_back(combined);
        }
    }
    
    tree_solutions* mergedObj = new tree_solutions;
    mergedObj->trees = merged;
    if(merged.empty()){
        mergedObj->lowerBound = INF;
        mergedObj->upperBound = INF;
        return mergedObj;
    }
    mergedObj->lowerBound = merged[0]->lowerBound;
    mergedObj->upperBound = merged[0]->upperBound;
    
    for (tree* t: merged){
        mergedObj->lowerBound = min(mergedObj->lowerBound, t->lowerBound);
        mergedObj->upperBound = min(mergedObj->upperBound, t->upperBound);
    }
    

    return mergedObj;
}
tree_solutions* calculate_smallest_misclassification(const vector<instance_t*> &insts, const vector<instance_t*> &globalInsts, int depth, int path[3], int upper_bound, const vector<int> feature_list) {
    
    
    int x[3] = {path[0], path[1], path[2]};
    sort(x, x + sizeof(x) / sizeof(x[0]));
    int key = (x[2] * 1000 + x[1]) * 1000 + x[0];

    
    if (mem.find(key) == mem.end()) {
        vector<tree*> t = {};
        tree* tr = new tree();
        int instances_true = 0;
        int instances_false = 0;

        cerr << "next seg" << endl;
        cerr << insts.size() << endl;
        
        
        for(instance_t* inst: insts){
            if(inst->label==0)
                instances_true++;
            else
                instances_false++;
        }
        tr->instances_true = instances_true;
        tr->instances_false = instances_false;
        tr->leaf_nodes = vector<tree*>(1,tr);
        int upperBound = min(instances_true,instances_false) + globalInsts.size();
        for(instance_t* inst: globalInsts){
            if(inst->label==0)
                instances_true++;
            else
                instances_false++;
        }
        int lowerBound = min(instances_false,instances_true);

        tr->lowerBound = lowerBound;
        tr-> upperBound = upperBound;
        t.push_back(tr);

        
        tree_solutions* leaf = new tree_solutions;
        leaf->trees = t;
        leaf->lowerBound = lowerBound;
        leaf->upperBound = upperBound;
        upper_bound = min(upperBound, upper_bound); 
        if (depth == 0) {
            
            mem.insert({key,leaf});
        } else {
            
            
            vector<tree_solutions*> viable_solutions {};
            
            
            viable_solutions.push_back(leaf);
            int lowest_bound_sofar = upper_bound;
            for (int i : feature_list) {
            
                
                vector<int> reduced_feature_list = {};
                for (int feature : feature_list) {
                    if (feature != i) {
                        reduced_feature_list.push_back(feature);
                    }
                }
                vector<instance_t*> left {};
                vector<instance_t*> right {};
                vector<instance_t*> globalLeft {};
                vector<instance_t*> globalRight {};

                cerr << insts.size() << endl;
                cerr << "done seg" << endl;
                for (instance_t* inst : insts) {
                    if (inst->features[i] == 0) {
                        left.push_back(inst);
                    }
                    else if(inst->features[i] == 1){
                        right.push_back(inst);
                    
                    } else {
                        globalLeft.push_back(inst);
                        globalRight.push_back(inst);
                    }
                }
                
                for (instance_t* inst : globalInsts) {
                    if (inst->features[i] == 0) {
                        globalLeft.push_back(inst);
                    }
                    else if(inst->features[i] == 1){
                        globalRight.push_back(inst);
                    
                    } else {
                        globalLeft.push_back(inst);
                        globalRight.push_back(inst);
                    }
                }
                path[depth] = i + 1;
                tree_solutions* left_misclassified = calculate_smallest_misclassification(left, globalLeft, depth - 1, path, upper_bound, reduced_feature_list);

                int min_misclassification = left_misclassified -> lowerBound;
                path[depth] = 0;
                path[depth] = i + 501;
                tree_solutions* right_misclassified = calculate_smallest_misclassification(right, globalRight, depth - 1, path, upper_bound - min_misclassification, reduced_feature_list);
                path[depth] = 0;
                tree_solutions* viable_solution_set = merge(insts, globalInsts,i, *left_misclassified, *right_misclassified);
                lowest_bound_sofar = min(viable_solution_set->lowerBound,lowest_bound_sofar);
                viable_solutions.push_back(viable_solution_set);
                upper_bound = min(viable_solution_set->upperBound, upper_bound);
            }
            tree_solutions* candidates = new tree_solutions;  
            candidates->upperBound = upper_bound;
            candidates->lowerBound = lowest_bound_sofar;

            for (tree_solutions* solution : viable_solutions) {
                
                filter(*candidates, *solution);

                delete solution;
            }
            mem.insert({key,candidates});
        }
    }
    return mem[key];
}

int main() {
    const string filename = "hypothyroid2.in";  
    vector<instance_t*> insts = readInstances(filename);
    insts = filter_instects(insts);
    int path[3] = {0, 0, 0};
    vector<instance_t*> globalInsts {};
    
    
    int totalFeatures = insts[0]->features.size();
    
    
    vector<int> feature_list {};
    for (int i = 0; i < totalFeatures; i++) {
        feature_list.push_back(i);
    }
    int depth = 3;
    tree_solutions* optimalTree = calculate_smallest_misclassification(insts,globalInsts, depth, path, INF/2, feature_list);
    printTree(optimalTree->trees[0],insts.size());
    cout << optimalTree->upperBound<< endl;
    cout.flush();
}