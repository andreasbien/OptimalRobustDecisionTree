#include <iostream>
#include <vector>
#include <stdio.h>
#include <bits/stdc++.h>

#include "tree.h"
#include "pushRelable.cpp"
#include "visualizeTree.cpp"
#include "readDatasets.cpp"
#include "instance_t.h"
#include <chrono>
using namespace std::chrono;

#include <map>
#include <limits>
#define INF INT32_MAX / 2

using namespace std;
vector<tree*> bruteForce(int depth, vector<int> features){
    vector<tree*> family = {};
    tree* leafTrue = new tree();
    tree* leafFalse = new tree();
    leafTrue->featureSplit = 0;
    leafFalse->featureSplit = 1;
    leafTrue->leaf_nodes.push_back(leafTrue);
    leafFalse->leaf_nodes.push_back(leafFalse);
    family.push_back(leafFalse);
    family.push_back(leafTrue);
    if(depth == 0){
        return family;
    }
    
    
    for(int i: features){
        if(depth == 2){
            cerr << i << endl;
        }
        vector<int> reduced_feature_list = {};
        for (int feature : features) {
            if (feature != i) {
                reduced_feature_list.push_back(feature);
            }
        }
        vector<tree*> left = bruteForce( depth-1, reduced_feature_list);
        vector<tree*> right = bruteForce( depth-1, reduced_feature_list);
        for(tree* t: left){
            for(tree* t2: right){
                tree* tr = new tree();
                tr->featureSplit = i + 1;
                tr->leftChild = t;
                tr->rightChild = t2;
                vector<tree*> leafs = t->leaf_nodes;
                leafs.insert(leafs.end(), t2->leaf_nodes.begin(), t2->leaf_nodes.end());
                tr->leaf_nodes = leafs;
                family.push_back(tr);
            }
        }
    }
    return family;
}

vector<int> getLeafConnections(instance_t* &inst, tree* t){
    if(t->leftChild == nullptr)
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



    

int evaluate(tree* t, vector<instance_t*> insts){
    vector<tree*> leafs = t->leaf_nodes;
    int misclassifications = 0;
    for(instance_t* inst: insts){
        vector<int> leafConnections = getLeafConnections(inst,t);
        for(int connection: leafConnections){
            if(leafs[connection]->featureSplit != inst->label){
                misclassifications++;
                break;
            }
        }
        
        
    }
    return misclassifications;


}
vector<instance_t*> filter_instects(vector<instance_t*> insts, int maxFeatures, int maxInstances) {
    // Height = amount of instances
    // Width = amount of features
    int height = insts.size();
    int width = insts[0]->features.size();

    vector<instance_t*> filtered_insts {};

    int c = 0;
    for (instance_t* inst : insts) {
        c++;
        if(c > maxInstances)
            break;
        instance_t* filtered_inst = new instance_t;
        *filtered_inst = {inst->label, {inst->features[0]}};
        filtered_insts.push_back(filtered_inst);
        
        // delete = inst;
    }

    int w = 0;
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

        w++;
        if(w > maxFeatures)
            break;
        for (int iy = 0; iy < maxInstances; iy++) {
            filtered_insts[iy]->features.push_back(insts[iy]->features[ix]);
        }
    }

    for (instance_t* inst : insts)
        delete inst;

    return filtered_insts;
}
/*
Randomly changes adversary_attack_power % of the features in the dataset to 2.
*/
void randomlyChangeFeatures(std::vector<instance_t*>& instances, float adversary_attack_power) {
    // Calculate the total number of features across all instances.
    size_t totalFeatureCount = 0;
    for ( auto& instance : instances) {
        totalFeatureCount += instance->features.size();
    }

    // Calculate the number of features to change.
    size_t featuresToChange = static_cast<size_t>(totalFeatureCount * adversary_attack_power);

    // Use a random number generator.
    std::default_random_engine generator;
    generator.seed(400);  // Use random device to seed the generator.
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

int main(){
    string filename = "heart-cleveland.in";  // Replace with your .in file
    vector<instance_t*> insts = readInstances(filename);


    int featureAmount = 20;
    int depth = 2;
    int instanceAmount = 100;
    float adversary_attack_power = 0.04;
    int height = insts.size();
    int width = insts[0]->features.size();
    // int depth;
    // int featureAmount;
    // int instanceAmount;
    // float adversary_attack_power;
    
    // string line;
    // vector<instance_t*> insts {};
    // cin >> featureAmount;
    // cin >> depth;
    // cin >> instanceAmount;
    // cin >> adversary_attack_power;
    // //cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // getline(cin, line);
    // getline(cin, line);

    // cerr << featureAmount << endl;
    // cerr << depth << endl;
    // cerr << instanceAmount << endl;
    // cerr << adversary_attack_power << endl;
    // cerr << line << endl;

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
    
    if(height < instanceAmount || insts[0]->features.size() < featureAmount){
        cout << -5 << endl;
        cout.flush();
    }
    else {
        insts = filter_instects(insts, featureAmount, instanceAmount);

    randomlyChangeFeatures(insts, adversary_attack_power);

    int totalFeatures = insts[0]->features.size();
    vector<int> feature_list {};
    for (int i = 0; i < totalFeatures; i++) {
        feature_list.push_back(i);
    }
    if(height < instanceAmount || insts[0]->features.size() < featureAmount){
        cout << -5 << endl;
        cout.flush();
    }
    else {
    // auto start = high_resolution_clock::now();


    
    vector<tree*> family = bruteForce(depth, feature_list);
    int lowest = INF;
    tree* bestTree = nullptr;
    cerr << family.size() << endl;
    int c = 0;
    for(tree* t: family){
        c++;
        if(c%1000 == 0)
            cerr << c << endl;
        int next = evaluate(t, insts);
        if(lowest > next){
            bestTree = t;
            lowest = next;
        }
        //cerr << c << endl;
    }
    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<microseconds>(stop - start);
    // cout << "time: "<< endl;
    // cout << duration.count() << endl;

    // for (tree* leaf: leafs) {
    //         cout << leaf->instances_true << " " << leaf->instances_false << endl;
    //     }
    cout << lowest << endl;
    cout.flush();
    }
    }
    
}