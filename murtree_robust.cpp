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




/**
 * A tree that represents a decision tree.
 */
// struct tree {
//     int featureSplit;
//     int lowerBound;
//     int upperBound;
//     tree* leftChild;// = new tree();
//     tree* rightChild;// = new tree();
//     //vector<int>* feature_path;
//     vector<tree*> leaf_nodes;

//     //number of instances labeled true
//     int instances_true;
//     //number of instances labeled false
//     int instances_false;
//     //vector<instance_t*>* global_instances;
// };
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
    // Height = amount of instances
    // Width = amount of features
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
map<long long, tree_solutions*> mem = {};
// map<int, vector<tree*>> mem = {};

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
void filter(tree_solutions &candidates, vector<tree_solutions*> solutions) {
    int upperBound = candidates.upperBound;
    // if(trees.empty()){
    //     candidates.lowerBound = INF;
    //     candidates.upperBound = INF;
    //     return;
    // }
    //for(int j = 0; j < 3; j++)
        // cerr << "upperBound" << endl;
    // cerr << upperBound << endl;
    if(upperBound == candidates.lowerBound){
        bool found = false;

        for(tree_solutions* solution: solutions){

        
        for (auto & t : solution->trees) {
        if (t->lowerBound <= upperBound) {
            if(!found){
                candidates.trees.push_back(t);
                found = true;
            }
            else
               delete t;
            }
        }
        delete solution;
        }
    }

    else{
        for(tree_solutions* solution: solutions){
            for (auto & t : solution->trees) {
            if (t->lowerBound < upperBound) {
                candidates.trees.push_back(t);
            }
            else{
                delete t;
            }
            }
            delete solution;
        }
    }

    // if(candidates.trees.empty()){
    //     candidates.lowerBound = INF;
    //     candidates.upperBound = INF;
    //     return;
    // }
    
}
//write function that filters out trees that are not viable

// void filter(vector<tree*> &candidates, vector<tree*> &solution) {
//     int upperBound = candidates->upperBound;
//     for (tree* t : solution) {
//         if (t->lowerBound < upperBound) {
//             candidates.push_back(t);
//         }
//     }
// }







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

        //copy elements into new vector
        std::vector<int> leafsBoth;
        leafsBoth.reserve(leafsLeft.size() + leafsRight.size());  
        leafsBoth.insert(leafsBoth.end(), leafsLeft.begin(), leafsRight.end());

        return leafsBoth;
    }


    

}
tree* combineTrees(const vector<instance_t*> insts,const vector<instance_t*> globalInsts, int feature, tree &left, tree &right){
    // cerr << "checkpointCombineTrees" << endl;
    tree* combined = new tree();
    combined->featureSplit = feature;
    combined->leftChild= &left;
    combined->rightChild = &right;
    // cerr << "checkpointCombineTrees0.1" << endl;
    // // cerr << left << endl;
    // cerr << right.lowerBound << endl;
    // // cerr << left->leaf_nodes << endl;
    vector<tree*> leftLeafNodes = left.leaf_nodes;
    vector<tree*> rightLeafNodes = right.leaf_nodes;
    // cerr << "checkpointCombineTrees0.1" << endl;

    // if(leftLeafNodes.empty()){
    //     leftLeafNodes = std::vector<tree*>(1,&left);
    // }
    // if(rightLeafNodes.empty()){
    //     rightLeafNodes = std::vector<tree*>(1,&right);
    // }
    
    vector<tree*> leafs(leftLeafNodes);
    // cout << "bullshit" << endl;
    // cerr << "checkpointCombineTrees0" << endl;
    leafs.insert(leafs.begin(),rightLeafNodes.begin(), rightLeafNodes.end());
    // cerr << "checkpointCombineTrees1" << endl;
    combined->leaf_nodes = leafs;
    
    Graph g(leafs.size()+2);
    int source = 0;
    int currentNode = 1;
    int sink = leafs.size() + 1;

    // cerr << "checkpoint3.1.1" << endl;
    for(tree* leaf: leafs){
        // cerr<< "leaf" << endl;
        // cerr <<leaf->instances_true << endl;
        // cerr <<leaf->instances_false << endl;
         g.addEdge(source,currentNode,leaf->instances_true);
         g.addEdge(currentNode, sink, leaf->instances_false);
        currentNode++;

    }

    // for(instance_t* inst: insts){
    //     vector<int> leafConnections = getLeafConnections(inst,combined);
    //     if(inst->label == 0){
    //         g.addEdge(source, leafConnections[0], 1);
    //         if(leafConnections.size() == 2)
    //             g.addEdge(leafConnections[0], leafConnections[1],1);
    //         if(leafConnections.size() > 2){
    //             int startNode = leafConnections[0];
    //                 g.addEdge(startNode,currentNode,1);
    //             for (size_t i = 1; i < leafConnections.size(); ++i) {
                    
    //                 g.addEdge(currentNode, i,1);
    //             }
    //             currentNode++;
    //         }
    //     }
    //     // // cerr << "checkpoint3.1.2" << endl;
    //     if(inst->label == 1){
    //         g.addEdge( leafConnections[0], sink, 1);
    //         if(leafConnections.size() == 2)
    //             g.addEdge(leafConnections[1], leafConnections[0],1);
    //         if(leafConnections.size() > 2){
    //             int startNode = leafConnections[0];
    //                 g.addEdge(currentNode,startNode,1);
    //             for (size_t i = 1; i < leafConnections.size(); ++i) {
                    
    //                 g.addEdge(i,currentNode,1);
    //             }
    //             currentNode++;
    //         }
    //     }
    // }
    // cerr << "checkpoint3.1.03" << endl;
    int upperBound = g.getMaxFlow(source, sink);
    // cerr << "checkpoint3.1.3" << endl;
    combined->upperBound = upperBound;
    combined->lowerBound = upperBound;
    // cerr << "" << endl;

    // cerr << "UPPERBOUND" << endl;
    // cerr << upperBound << endl;
    // cerr << "UPPERBOUND" << endl;
    // cerr << "" << endl;

    return combined;

}

tree_solutions* merge(const vector<instance_t*> insts,const vector<instance_t*> globalInsts, int feature, tree_solutions &left, tree_solutions &right){
    // cerr << "checkpoint3.2" << endl;
    vector<tree*> merged = {};

    // cerr << right.trees.size() << endl;
    // cerr.flush();
    // cerr << left.trees.size() << endl;
    // cerr.flush();
    int c = 0;
    int a = 0;
    for (auto & r : right.trees) {
        // cerr << "a" << endl;
        // cerr << a << endl;
        a++;
        for (auto & l : left.trees) {
            // cerr << "c" << endl;
            // cerr << c << endl;
            c++;
            // cerr << right.trees.size() << endl;
            // cerr.flush();
            // cerr << left.trees.size() << endl;
            // cerr.flush();
            // if(c == 1)
            //     break;
            //cerr << "checkpoint3.2" << endl;
            //printTree(l,0);
            //printTree(r,0);
            tree* combined = combineTrees(insts,globalInsts,feature, *l,*r);

            //printTree(combined,0);
            
            merged.push_back(combined);
            // cerr << "checkpoint3.2.1" << endl;
        }
    }
    // cerr << "checkpoint3.2.14" << endl;
    tree_solutions* mergedObj = new tree_solutions;
    // cerr << "checkpoint3.2.15" << endl;
    mergedObj->trees = merged;
    // cerr << "checkpoint3.2.2" << endl;
    if(merged.empty()){
        mergedObj->lowerBound = INF;
        mergedObj->upperBound = INF;
        return mergedObj;
    }
    mergedObj->lowerBound = merged[0]->lowerBound;
    mergedObj->upperBound = merged[0]->upperBound;
    // cerr << "checkpoint3.2.3" << endl;
    for (tree* t: merged){
        mergedObj->lowerBound = min(mergedObj->lowerBound, t->lowerBound);
        mergedObj->upperBound = min(mergedObj->upperBound, t->upperBound);
    }
    // cerr << "checkpoint3.3" << endl;

    return mergedObj;
}






tree_solutions* calculate_smallest_misclassification(const vector<instance_t*> insts, const vector<instance_t*> globalInsts, int depth, long long path[5], int upper_bound, const vector<int> feature_list) {
    //cout << "!asdf";
    // Sort the numbers given in path to make a unique key/ID of the current branch
    long long x[5] = {path[0], path[1], path[2], path[3], path[4]};
    sort(x, x + sizeof(x) / sizeof(x[0]));
    long long key = (((x[4] * 1000 + x[3]) * 1000 + x[2])*1000 + x[1]) * 1000 + x[0];

    // If the key doesn't appear (the current branch hasn't been computed yet), compute the misclassification for the current branch
    if (mem.find(key) == mem.end()) {
        // Calculate the amount of 0-labeled instances and 1-labeled instances
        // int c[2] {0, 0};
        // for (instance_t* inst : insts) {
        //     c[inst->label]++;
        // }

        // Take the current best to be the least amount of misclassifications we could get if this were a leaf node
        //int best = min(c[0], c[1]);
        //upper_bound = min(upper_bound, best);


        // first consider solution where node is a leaf
        
        vector<tree*> t = {};
        tree* tr = new tree();
        int instances_true = 0;
        int instances_false = 0;

        //cerr << "next seg" << endl;
        //cerr << insts.size() << endl;
        // if(insts.size() > 0)
        //     cerr << insts[0]->features.size() << endl;
        for(instance_t* inst: insts){
            if(inst->label==0)
                instances_true++;
            else
                instances_false++;
        }
        // cerr << "check" << endl;
        
        // cerr << instances_true << endl;
        // cerr << instances_false << endl;
        // cerr << "check" << endl;
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
        // cerr << "leaf" << endl;
        
        // cerr << tr->instances_true << endl;
        // cerr << tr->instances_false << endl;
        // cerr << "leaf" << endl;
        t.push_back(tr);

        //tr->global_instances = globalInsts;
        tree_solutions* leaf = new tree_solutions;
        leaf->trees = t;
        leaf->lowerBound = lowerBound;
        leaf->upperBound = upperBound;
        upper_bound = min(upperBound, upper_bound); 
        if (depth == 0) {
            //if depth == 0, then node must be a leaf.
            mem.insert({key,leaf});
        } else {
            
            // Width = amount of features
            vector<tree_solutions*> viable_solutions {};
            //ensure that leaf solution is included as possible solutions
            //if(leaf->lowerBound <= upper_bound )
            viable_solutions.push_back(leaf);
            int lowest_bound_sofar = upper_bound;
            for (int i : feature_list) {
            //for (int i = 0; i < width; i++) {
                // Create a new list of features that excludes the current feature
                vector<int> reduced_feature_list = {};
                for (int feature : feature_list) {
                    if (feature != i) {
                        reduced_feature_list.push_back(feature);
                    }
                }
                // Some printing to stderr, just to make the progress visual in the terminal
                // if (depth == 3) {
                //     // cerr << "#";
                // } else {
                //     // cerr << endl;
                // }

                // Split the instances based on the chosen feature
                vector<instance_t*> left {};
                vector<instance_t*> right {};
                vector<instance_t*> globalLeft {};
                vector<instance_t*> globalRight {};

                //cerr << insts.size() << endl;
                //if(insts.size() > 0)
                // if(insts == NULL)
                //     continue;
                    //cerr << insts[0]->features.size() << endl;
                //cerr << "done seg" << endl;
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
                // cerr << "checkpoint1" << endl;
                // Update the path (i + 1 will mean "all instances have feature i set to 0"),
                // and then recurse into the left child
                // In the end, reset the path
                // the upper bound is set to (upperBound - 1), as any better solution must be at least 1 better than the current best
                path[depth] = i + 1;
                tree_solutions* left_misclassified = calculate_smallest_misclassification(left, globalLeft, depth - 1, path, INF, reduced_feature_list);

                int min_misclassification = left_misclassified -> lowerBound;
                // If the left child is already at least as high as the upper bound, don't bother recursing into the right child
                // cerr << "checkpoint2" << endl;
                //check 

                // if(depth == 2){
                //     cerr << insts[0]->features.size() << endl;
                //     cerr << reduced_feature_list.size() << endl;
                //     cerr << i << endl;
                //     cerr << "left and right:" << endl;
                //     cerr << left.size() << endl;
                //     cerr << right.size() << endl;
                //     cerr << "left and right:" << endl;
                //     cerr.flush();
                // }

                // if(min_misclassification > upper_bound){
                //     // for(tree* t: left_misclassified->trees)
                //     //     delete t;
                //     // delete left_misclassified;
                //     continue;
                // }
                 
                // cerr << "checkpoint2.1" << endl;
                
                
                path[depth] = 0;


                // Some printing to stderr, just to make the progress visual in the terminal
                if (depth == 4) {
                    // cerr << "|";
                }

                

                // Update the path (i + 501 will mean "all instances have feature i set to 1"),
                // and then recurse into the right child
                // In the end, reset the path
                // Note that the new upper bound is set to (upper_bound - left_misclassified), to take the misclassification of the left side into account as well
                path[depth] = 501 + i;
                tree_solutions* right_misclassified = calculate_smallest_misclassification(right, globalRight, depth - 1, path, /*upper_bound - min_misclassification*/ INF, reduced_feature_list);
                path[depth] = 0;
                // cerr << "checkpoint3" << endl;
                // cerr << right_misclassified->trees.size() << endl;
                // cerr << left_misclassified->trees.size() << endl;
                
                    // cerr << "checkpoint3.1" << endl;
                    // cerr << "left_misclassified->trees.empty() || right_misclassified->trees.empty()" << endl;
                    // cerr << left_misclassified->trees.empty() << endl;
                    // cerr << right_misclassified->trees.empty() << endl;
                // if(left_misclassified->trees.empty() || right_misclassified->trees.empty())
                //     continue;
                tree_solutions* viable_solution_set = merge(insts, globalInsts,i, *left_misclassified, *right_misclassified);
                
                    // printTree(viable_solution_set->trees[0], i);
                // cerr << "checkpoint3.1" << endl;
                lowest_bound_sofar = min(viable_solution_set->lowerBound,lowest_bound_sofar);
                viable_solutions.push_back(viable_solution_set);
                upper_bound = min(viable_solution_set->upperBound, upper_bound);
                
                /*
                upperBound = MAX;
                vector<tree*> viable_solutions {};
                for(treeLeft:left_misclassified){
                    for(treeRight:right_misclassified){
                        bounds = merge(treeLeft,treeRight,upperBound);
                        if(bounds.viable)
                            viable_solutions.push_back(combineTree(treeLeft,treeRight))
                        upperBound = min(upperBound,bounds.upper);
                    }
                
                }
                viable_solutions = filter(viable_solutions, upperBound)
                */
                // Check whether splitting on the current feature improved the number of misclassifications
                // if (best > misclassified) {
                //     // If so, update best and recompute the upper bound
                //     best = misclassified;
                //     upper_bound = min(upper_bound, best);

                //     // If we have no misclassifications, we cannot improve any more, and can stop checking splits
                //     if (best == 0) {
                //         break;
                //     }
                // }
            }
            //add solution where this is just a leaf node:


            // cerr << "checkpoint3.5" << endl;
            // cerr << "lowest_bound_sofar" << endl;
            //tree_solutions candidatesObj;  // Create a valid object
            // if(viable_solutions.empty()){
            //         tree_solutions* bullshitSolution = new tree_solutions;
            //         bullshitSolution->lowerBound = INF;
            //         bullshitSolution->upperBound = INF;
            //         bullshitSolution->trees = {};
            //         tree* bullshitTree = new tree();
            //         bullshitTree->lowerBound = INF;
            //         bullshitTree->upperBound = INF;
            //         bullshitSolution->trees.push_back(bullshitTree);
            //         return bullshitSolution;
            //     }
            tree_solutions* candidates = new tree_solutions;  // Initialize pointer with valid object
            candidates->upperBound = lowest_bound_sofar;
            candidates->lowerBound = lowest_bound_sofar;
            // for (tree_solutions* solution : viable_solutions) {
                
            //     filter(*candidates, *solution);

            //     delete solution;
            // }
            filter(*candidates, viable_solutions);
            if(candidates->trees.empty()){
                candidates->lowerBound = INF;
                candidates->upperBound = INF;
                //return candidates;
            }
            //delete viable_solutions;
            //cout << "lower bound: "	;
            // cerr << candidates->lowerBound << endl;
            // cerr << depth << endl;
            // sort(candidates->trees.begin(), candidates->trees.end(), compareByUpperBound);
            // cerr << "checkpoint4" << endl;
            
            mem.insert({key,candidates});

        }

        // Cache the calculated value
        //mem.insert({key, best});
    }

    // Return the cached value
    
    //cout << mem[key]->lowerBound << endl;
    // cerr << "checkpoint5" << endl;
    // cerr << mem[key]->upperBound << endl;
    return mem[key];
}

int main() {
    // const string filename = "soybean3.in";  // Replace with your .in file
    // vector<instance_t*> insts = readInstances(filename);
    // int depth = 3;

    // Print out the instances
    // for (const auto& instance : instances) {
    //     //cout << "Label: " << instance->label << ", Features: ";
    //     for (const auto& feature : instance->features) {
    //         cout << feature << ' ';
    //     }
    //     cout << '\n';
    //     delete instance; // delete the instance once it's processed to avoid memory leak
    // }

    // Read input, parse instances
    //cout << "!";
    int depth;
    string line;
    vector<instance_t*> insts {};

    cin >> depth;
    getline(cin, line);
    getline(cin, line);

    int width = line.size() / 2;
    int height = 0;
    while (line.size() > 0) {
        vector<int> attr {};
        for (int ix = 0; ix < width; ix++)
            attr.push_back(line[2 + 2 * ix] - '0');
        instance_t* inst = new instance_t;
        *inst = {line[0] - '0', attr};
        insts.push_back(inst);

        getline(cin, line);
        height++;
    }

    // Preprocess dataset, remove unnecessary features
    insts = filter_instects(insts);

    // vector<instance_t*> insts {};
    // int depth = 2;
    // instance_t* inst1 = new instance_t;
    // inst1->label = 0;
    // inst1->features = {0, 0, 0};
    // instance_t* inst2 = new instance_t;
    // inst2->label = 0;
    // inst2->features = {1, 0, 0};
    // instance_t* inst3 = new instance_t;
    // inst3->label = 0;
    // inst3->features = {0, 1, 1};
    // instance_t* inst4 = new instance_t;
    // inst4->label = 1;
    // inst4->features = {1, 1, 1};

    // insts.push_back(inst1);
    // insts.push_back(inst2);
    // insts.push_back(inst3);
    // insts.push_back(inst4);

    // Calculate the smallest misclassification starting from the root
    long long path[5] = {0, 0, 0, 0, 0};
    vector<instance_t*> globalInsts {};
    //cout << depth << endl;
    //depth = 1;
    int totalFeatures = insts[0]->features.size();
    //cerr << "totalFeatures" << endl;
    //cerr << totalFeatures << endl;
    vector<int> feature_list {};
    for (int i = 0; i < totalFeatures; i++) {
        feature_list.push_back(i);
    }
    //int depth = 3;
    cerr << insts.size() << endl;
    tree_solutions* optimalTree = calculate_smallest_misclassification(insts,globalInsts, depth, path, INF/2, feature_list);
    //printTree(optimalTree->trees[0], optimalTree->upperBound);
    // tree* first = optimalTree->trees[0];
    // cerr << depth << endl;
    // cerr << "lower bound" << endl;
    // Print answer
    //// cerr << optimalTree->lowerBound;
    
    // cerr << depth << endl;
    //cout << optimalTree->trees << endl;
    // cerr << "number samples" << endl;
    // cerr << insts.size() << endl;
    
    // cerr << "score" << endl;
    // cerr << optimalTree->upperBound << endl;

    // Print the tree
    // cerr << "tree representation:" << endl;
    //cerr <<  << endl;
    printTree(optimalTree->trees[0],optimalTree->trees.size());
    //cout.flush();
    //cerr.flush();
    //// cerr << optimalTree->trees.front->leftChild.instances_true << endl;
    cout << optimalTree->upperBound<< endl;
    cout.flush();
}