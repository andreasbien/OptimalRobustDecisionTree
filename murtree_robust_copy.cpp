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






struct tree_solutions {
    int lowerBound;
    int local_lowerBound;
    int upperBound;
    vector<tree*> trees = {};
};

bool compareByLowerBound(const tree* a, const tree* b) {
    return a->lowerBound < b->lowerBound;
}


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

/*
Randomly changes 5% of the features in the dataset to 2.
*/
void randomlyChangeFeatures(std::vector<instance_t*>& instances) {
    // Calculate the total number of features across all instances.
    size_t totalFeatureCount = 0;
    for (const auto& instance : instances) {
        totalFeatureCount += instance->features.size();
    }

    // Calculate the number of features to change.
    size_t featuresToChange = static_cast<size_t>(totalFeatureCount * 0.15);

    // Use a random number generator.
    std::default_random_engine generator;
    generator.seed(std::random_device{}());  // Use random device to seed the generator.
    std::uniform_int_distribution<size_t> instanceDistribution(0, instances.size() - 1);

    for (size_t i = 0; i < featuresToChange; ++i) {
        // Randomly select an instance.
        size_t instanceIndex = instanceDistribution(generator);
        instance_t* instance = instances[instanceIndex];

        // Ensure the instance has at least one feature.
        if (!instance->features.empty()) {
            // Select a random feature from this instance.
            std::uniform_int_distribution<size_t> featureDistribution(0, instance->features.size() - 1);
            size_t featureIndex = featureDistribution(generator);

            // Change the feature to 2.
            instance->features[featureIndex] = 2;
        }
    }
}

/**
 * The memoization map that will map a certain branch (represented
 * by an integer key) to the amount of misclassifications that we
 * found for it.
 */
map<long long, tree_solutions*> mem = {};

map<long long, int> memCheck = {};
// map<int, vector<tree*>> mem = {};
bool compareByPath(const int& feature1,const int& feature2, int depth,long long path[5]) {
    path[depth] = 0;
    path[depth] = feature1 + 1;
    long long key = (((path[4] * 1000 + path[3]) * 1000 + path[2])*1000 + path[1]) * 1000 + path[0];
    
    path[depth] = 0;
    path[depth] = feature1 + 501;
    long long key2 = (((path[4] * 1000 + path[3]) * 1000 + path[2])*1000 + path[1]) * 1000 + path[0];
    
    path[depth] = 0;
    path[depth] = feature2 + 1;
    long long key3 = (((path[4] * 1000 + path[3]) * 1000 + path[2])*1000 + path[1]) * 1000 + path[0];
    path[depth] = 0;
    if(mem.find(key) == mem.end())
        if(mem.find(key3) == mem.end())
            return mem.find(key2) != mem.end();
        else
            return false;
    else
        if(mem.find(key3) == mem.end())
            true;
        else
            return mem.find(key2) == mem.end();

    return true;
}
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

    
}







vector<int> getLeafConnections(instance_t* &inst, tree* t){
    if(t->featureSplit == 0)
        return std::vector<int>(1,0);
    if(inst->features[t->featureSplit-1] == 0)
        return getLeafConnections(inst,t->leftChild);
    else if(inst->features[t->featureSplit-1] == 1){
        vector<int> leafsRight = getLeafConnections(inst,t->rightChild);
        int leafsInLeft = t->leftChild->leaf_nodes.size();
        std::transform(leafsRight.begin(), leafsRight.end(), leafsRight.begin(),
                   [leafsInLeft](int element) { return element + leafsInLeft; });
        return leafsRight;
    }
    else{
        vector<int> leafsLeft = getLeafConnections(inst,t->leftChild);
        vector<int> leafsRight = getLeafConnections(inst,t->rightChild);
        int leafsInLeft = t->leftChild->leaf_nodes.size();
        std::transform(leafsRight.begin(), leafsRight.end(), leafsRight.begin(),
                  [leafsInLeft](int element) { return element + leafsInLeft; });

        leafsLeft.reserve(leafsLeft.size() + leafsRight.size());  
        leafsLeft.insert(leafsLeft.end(), leafsRight.begin(), leafsRight.end());


        return leafsLeft;
    }


    

}
struct Edgy{
    int from;
    int to;
    int capacity;
    Edgy(int from, int to, int capacity){
        this->from = from;
        this->to = to;
        this->capacity = capacity;
    }
};
tree* combineTrees(const vector<instance_t*> insts,const vector<instance_t*> globalInsts, int feature, tree &left, tree &right){
    tree* combined = new tree();
    combined->featureSplit = feature;
    combined->leftChild= &left;
    combined->rightChild = &right;

    vector<tree*> leftLeafNodes = left.leaf_nodes;
    vector<tree*> rightLeafNodes = right.leaf_nodes;

    
    vector<tree*> leafs(leftLeafNodes);
    leafs.insert(leafs.end(),rightLeafNodes.begin(), rightLeafNodes.end());
    combined->leaf_nodes = leafs;
        
    int currentNode = 1;
    vector<int> fromSource(leafs.size(), 0);
    vector<int> toSink(leafs.size(), 0);
    vector<vector<int>> leafToLeaf(leafs.size(), vector<int>(leafs.size(), 0));
    vector<Edgy> edges = {};
    for(tree* leaf: leafs){
        fromSource[currentNode-1] = leaf->instances_true;
        toSink[currentNode-1] = leaf->instances_false;
        currentNode++;

    }
    for(instance_t* inst: insts){
        vector<int> leafConnections = getLeafConnections(inst,combined);
        if(leafConnections.size() == 1)
            continue;
        if(inst->label == 0){
            
            fromSource[leafConnections[0]]++;
            if(leafConnections.size() == 2)
                leafToLeaf[leafConnections[0]][leafConnections[1]]++;
            if(leafConnections.size() > 2){
                int startNode = leafConnections[0];
                edges.push_back(Edgy(startNode,currentNode,1));
                for (size_t i = 1; i < leafConnections.size(); i++) {
                    edges.push_back(Edgy(currentNode,i,1));
                }
                currentNode++;
            }
        }
        if(inst->label == 1){
            toSink[leafConnections[0]]++;
            if(leafConnections.size() == 2)
                leafToLeaf[leafConnections[1]][leafConnections[0]]++;
            if(leafConnections.size() > 2){
                edges.push_back(Edgy(currentNode,leafConnections[0],1));
                for (size_t i = 1; i < leafConnections.size(); ++i) {
                    edges.push_back(Edgy(i,currentNode,1));
                }
                currentNode++;
            }
        }
        
    }

    
    int source = 0;
    int sink = currentNode;
    Graph g = Graph(currentNode+1);

    for(int i = 0; i < leafs.size(); i++){
        if(fromSource[i] > 0)
            g.addEdge(source, i + 1, fromSource[i]);
        if(toSink[i] > 0)
            g.addEdge(i + 1, sink, toSink[i]);
        
        for(int j = 0; j < leafs.size(); j++){
            if(leafToLeaf[i][j] > 0)
                g.addEdge(i+1, j+1, leafToLeaf[i][j]);
        }
    }
    for(Edgy e: edges){
        g.addEdge(e.from, e.to, e.capacity);
    }

    combined->upperBound = g.getMaxFlow(source, sink) + globalInsts.size();
    combined->local_lowerBound = combined->upperBound - globalInsts.size();

    for(instance_t* inst: globalInsts){
        vector<int> leafConnections = getLeafConnections(inst,combined);
        if(leafConnections.size() == 1)
            continue;
        if(inst->label == 0){
            fromSource[leafConnections[0]]++;
            if(leafConnections.size() == 2)
                leafToLeaf[leafConnections[0]][leafConnections[1]]++;
            if(leafConnections.size() > 2){
                int startNode = leafConnections[0];
                edges.push_back(Edgy(startNode,currentNode,1));
                for (size_t i = 1; i < leafConnections.size(); i++) {
                    edges.push_back(Edgy(currentNode,i,1));
                }
                currentNode++;
            }
        }
        if(inst->label == 1){
            toSink[leafConnections[0]]++;
            if(leafConnections.size() == 2)
                leafToLeaf[leafConnections[1]][leafConnections[0]]++;
            if(leafConnections.size() > 2){
                edges.push_back(Edgy(currentNode,leafConnections[0],1));
                for (size_t i = 1; i < leafConnections.size(); ++i) {
                    edges.push_back(Edgy(i,currentNode,1));
                }
                currentNode++;
            }
        }
        
    }

    sink = currentNode;
    Graph gLower = Graph(currentNode+1);
    for(int i = 0; i < leafs.size(); i++){
        if(fromSource[i] > 0)
            gLower.addEdge(source, i + 1, fromSource[i]);
        if(toSink[i] > 0)
            gLower.addEdge(i + 1, sink, toSink[i]);
        
        for(int j = 0; j < leafs.size(); j++){
            if(leafToLeaf[i][j] > 0)
                gLower.addEdge(i + 1, j + 1, leafToLeaf[i][j]);
        }
    }
    for(Edgy e: edges){
        gLower.addEdge(e.from, e.to, e.capacity);
    }
    combined->lowerBound = gLower.getMaxFlow(source, sink);

    return combined;

}

tree_solutions* merge(const vector<instance_t*> insts,const vector<instance_t*> globalInsts, int feature, tree_solutions &left, tree_solutions &right){
    vector<tree*> merged = {};
    tree_solutions* mergedObj = new tree_solutions;
    mergedObj->upperBound = INF;
    mergedObj->lowerBound = INF;
    mergedObj->local_lowerBound = INF;
    for (auto & r : right.trees)
        for (auto & l : left.trees)
            mergedObj->upperBound = min(mergedObj->upperBound, l->local_lowerBound + r->upperBound);
    for (auto & r : right.trees) {
        for (auto & l : left.trees) {
            if(l->local_lowerBound + r->lowerBound >= mergedObj->upperBound)
                continue;
            if(r->local_lowerBound + l->lowerBound >= mergedObj->upperBound)
                continue;
            tree* combined = combineTrees(insts,globalInsts,feature, *l,*r);
            mergedObj->local_lowerBound = min(mergedObj->local_lowerBound, combined->local_lowerBound);
            mergedObj->lowerBound = min(mergedObj->lowerBound, combined->lowerBound);
            mergedObj->upperBound = min(mergedObj->upperBound, combined->upperBound);
            merged.push_back(combined);
        }
    }
    
    mergedObj->trees = merged;

    return mergedObj;
}






tree_solutions* calculate_smallest_misclassification(const vector<instance_t*> insts, const vector<instance_t*> globalInsts, int depth, long long path[5], int upper_bound, vector<int> feature_list) {
    // Sort the numbers given in path to make a unique key/ID of the current branch
    
    long long x[5] = {path[0], path[1], path[2], path[3], path[4]};
    sort(x, x + sizeof(x) / sizeof(x[0]));
    long long key = (((x[4] * 1000 + x[3]) * 1000 + x[2])*1000 + x[1]) * 1000 + x[0];
    if(memCheck[key] >= upper_bound){
        return nullptr;
    }
    // If the key doesn't appear (the current branch hasn't been computed yet), compute the misclassification for the current branch
    if (mem.find(key) == mem.end()) {

        // first consider solution where node is a leaf
        
        vector<tree*> t = {};
        tree* tr = new tree();
        int instances_true = 0;
        int instances_false = 0;


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
        leaf-> local_lowerBound = lowerBound;
        leaf->upperBound = upperBound;
        upper_bound = min(upperBound, upper_bound); 
        if (depth == 0) {
            //if depth == 0, then node must be a leaf.
            mem.insert({key,leaf});
        } else {
            
            // Width = amount of features
            vector<tree_solutions*> viable_solutions {};
            //ensure that leaf solution is included as possible solutions
            if(leaf->lowerBound <= upper_bound ){
                viable_solutions.push_back(leaf);
            }
            else
                delete leaf;
            int lowest_bound_sofar = upper_bound;
            int lowest_local_bound_sofar = upper_bound;
            std::sort(feature_list.begin(), feature_list.end(), [depth, path](const int& a, const int& b) {
                return compareByPath(a, b, depth, path);
            });
            for (int i : feature_list) {
                if(depth == 2)
                    cerr << i << endl;
            //for (int i = 0; i < width; i++) {
                // Create a new list of features that excludes the current feature
                vector<int> reduced_feature_list = {};
                for (int feature : feature_list) {
                    if (feature != i) {
                        reduced_feature_list.push_back(feature);
                    }
                }


                // Split the instances based on the chosen feature
                vector<instance_t*> left {};
                vector<instance_t*> right {};
                vector<instance_t*> globalLeft {};
                vector<instance_t*> globalRight {};

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
                // Update the path (i + 1 will mean "all instances have feature i set to 0"),
                // and then recurse into the left child
                // In the end, reset the path
                // the upper bound is set to (upperBound - 1), as any better solution must be at least 1 better than the current best
                path[depth] = i + 1;
                tree_solutions* left_misclassified = calculate_smallest_misclassification(left, globalLeft, depth - 1, path, upper_bound, reduced_feature_list);
                if(left_misclassified == nullptr){
                    path[depth] = 0;
                    continue;
                }

                int min_misclassification = left_misclassified -> lowerBound;
                int local_min_misclassification = left_misclassified -> local_lowerBound;
                if(min_misclassification > upper_bound){
                    path[depth] = 0;
                    continue;
                    
                }
                // If the left child is already at least as high as the upper bound, don't bother recursing into the right child
                path[depth] = 0;

                // Update the path (i + 501 will mean "all instances have feature i set to 1"),
                // and then recurse into the right child
                // In the end, reset the path
                // Note that the new upper bound is set to (upper_bound - left_misclassified), to take the misclassification of the left side into account as well
                path[depth] = 501 + i;
                tree_solutions* right_misclassified = calculate_smallest_misclassification(right, globalRight, depth - 1, path, upper_bound - local_min_misclassification, reduced_feature_list);
                if(right_misclassified == nullptr){
                    path[depth] = 0;
                    continue;
                }
                path[depth] = 0;
                tree_solutions* viable_solution_set = merge(insts, globalInsts,i + 1, *left_misclassified, *right_misclassified);
                lowest_bound_sofar = min(viable_solution_set->lowerBound,lowest_bound_sofar);
                lowest_local_bound_sofar = min(viable_solution_set->local_lowerBound,lowest_local_bound_sofar);
                viable_solutions.push_back(viable_solution_set);
                upper_bound = min(viable_solution_set->upperBound, upper_bound);
                
                
            }


            if(viable_solutions.empty()){
                memCheck.insert({key,upper_bound});
                return nullptr;
            }
            
            tree_solutions* candidates = new tree_solutions; 
            candidates->upperBound = upper_bound;
            candidates->lowerBound = lowest_bound_sofar;
            candidates->local_lowerBound = lowest_local_bound_sofar;

            filter(*candidates, viable_solutions);
            if(candidates->trees.empty()){
                delete candidates;
                memCheck.insert({key,upper_bound});
                return nullptr;
            }
            std::sort(candidates->trees.begin(), candidates->trees.end(), compareByLowerBound);            
            mem.insert({key,candidates});
            return candidates;
        }
    }

    // Return the cached value
    return mem[key];
}

int main() {
    const string filename = "soybean3.in";  // Replace with your .in file
    vector<instance_t*> insts = readInstances(filename);
    int depth = 2;

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
    // int depth;
    // string line;
    // vector<instance_t*> insts {};

    // cin >> depth;
    // getline(cin, line);
    // getline(cin, line);

    // int width = line.size() / 2;
    // int height = 0;
    // while (line.size() > 0) {
    //     vector<int> attr {};
    //     for (int ix = 0; ix < width; ix++)
    //         attr.push_back(line[2 + 2 * ix] - '0');
    //     instance_t* inst = new instance_t;
    //     *inst = {line[0] - '0', attr};
    //     insts.push_back(inst);

    //     getline(cin, line);
    //     height++;
    // }

    // Preprocess dataset, remove unnecessary features
    insts = filter_instects(insts);
    randomlyChangeFeatures(insts);

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
    int totalFeatures = insts[0]->features.size();
    vector<int> feature_list {};
    for (int i = 0; i < totalFeatures; i++) {
        feature_list.push_back(i);
    }
    //int depth = 3;
    cerr << insts.size() << endl;
    tree_solutions* optimalTree = calculate_smallest_misclassification(insts,globalInsts, depth, path, INF/2, feature_list);
    tree* opt = optimalTree->trees[0];
    tree* doubleCheck = combineTrees(insts,globalInsts,opt->featureSplit, *opt->leftChild, *opt->rightChild);
    cerr << "doubleCheck->lowerBound" << endl;
    cerr << doubleCheck->lowerBound << endl;
    cerr << "doubleCheck->upperBound" << endl;
    cerr << doubleCheck->upperBound << endl;

    printTree(optimalTree->trees[0],optimalTree->trees.size());
    
    cout << optimalTree->lowerBound << endl;
    cout << optimalTree->upperBound<< endl;
    cout.flush();
}