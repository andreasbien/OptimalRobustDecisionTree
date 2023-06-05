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

#define INF INT32_MAX / 2

using namespace std;

/**
 * An instance that represents one row in a dataset.
 * Every instance has a set of binary features and a binary label.
 */
struct instance_t {
    int label;
    vector<int> features;
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
map<int, int> mem = {};
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
int calculate_smallest_misclassification(const vector<instance_t*> &insts, int depth, int path[3], int upper_bound) {
    // Sort the numbers given in path to make a unique key/ID of the current branch
    int x[3] = {path[0], path[1], path[2]};
    sort(x, x + sizeof(x) / sizeof(x[0]));
    int key = (x[2] * 1000 + x[1]) * 1000 + x[0];

    // If the key doesn't appear (the current branch hasn't been computed yet), compute the misclassification for the current branch
    if (mem.find(key) == mem.end()) {
        // Calculate the amount of 0-labeled instances and 1-labeled instances
        int c[2] {0, 0};
        for (instance_t* inst : insts) {
            c[inst->label]++;
        }

        // Take the current best to be the least amount of misclassifications we could get if this were a leaf node
        int best = min(c[0], c[1]);
        upper_bound = min(upper_bound, best);

        if (c[0] == 0 || c[1] == 0) {
            // If all instances are of one type, we have no misclassifications
            cerr << "Depth = " << depth << endl;
            cerr << path[0] << " " << path[1] << " " << path[2] << endl;
            cerr.flush();
            best = 0;
        } else if (depth == 0) {
            best = min(c[0], c[1]);
            cerr << "Depth 0, best = " << best << endl;
        } else if (depth == 2) {
            // Specialized depth-2 algorithm

            // Width = amount of features
            int width = insts[0]->features.size();

            // fq[label][feat_i][feat_j] denotes the amount of instances with label <label> for which feature <feat_i> and feature <feat_j> are both 1
            // fq[label][feat_x][feat_x], by extension, denotes the instances with label <label> for which feature <feat_x> is 1
            int fq[2][width][width];

            // Initialize everything to 0
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < width; j++) {
                    for (int k = 0; k < width; k++) {
                        fq[i][j][k] = 0;
                    }
                }
            }

            // Calculate every fq-value by iterating over every instance and every [feat_i][feat_j] pair
            for (instance_t* inst : insts) {
                for (int i = 0; i < width; i++) {
                    if (inst->features[i]) {
                        fq[inst->label][i][i]++;

                        for (int j = i + 1; j < width; j++) {
                            if (inst->features[j]) {
                                fq[inst->label][i][j]++;
                                fq[inst->label][j][i]++;
                            }
                        }
                    }
                }
            }

            // Loop over every feature we could filter on in the parent node
            for (int i = 0; i < width; i++) {
                // Keep track of the best left-child-score and right-child-score if we were to filter on feature i in the parent
                int best_left = INF;
                int best_right = INF;

                for (int j = 0; j < width; j++) {
                    // Don't filter on the same feature both in the parent and the children
                    if (i == j)
                        continue;

                    // Calculate the best left and right score if we were filtering on feature j in the children (refer to Emir's paper for these formulas)
                    int left = min(fq[0][j][j] - fq[0][i][j], fq[1][j][j] - fq[1][i][j])
                             + min(c[0] - fq[0][i][i] - fq[0][j][j] + fq[0][i][j], c[1] - fq[1][i][i] - fq[1][j][j] + fq[1][i][j]);
                    best_left = min(best_left, left);

                    int right = min(fq[0][i][j], fq[1][i][j])
                              + min(fq[0][i][i] - fq[0][i][j], fq[1][i][i] - fq[1][i][j]);
                    best_right = min(best_right, right);
                }

                // Combine the lowest misclassifications on either side
                best = min(best, best_left + best_right);
            }
        } else {
            
            // Width = amount of features
            int width = insts[0]->features.size();

            for (int i = 0; i < width; i++) {
                // Some printing to stderr, just to make the progress visual in the terminal
                if (depth == 3) {
                    cerr << "#";
                } else {
                    cerr << endl;
                }

                // Split the instances based on the chosen feature
                vector<instance_t*> left {};
                vector<instance_t*> right {};
                for (instance_t* inst : insts) {
                    if (inst->features[i] == 0) {
                        left.push_back(inst);
                    } else {
                        right.push_back(inst);
                    }
                }

                // Update the path (i + 1 will mean "all instances have feature i set to 0"),
                // and then recurse into the left child
                // In the end, reset the path
                path[depth - 2] = i + 1;
                int left_misclassified = calculate_smallest_misclassification(left, depth - 1, path, best);
                
                path[depth - 2] = 0;

                // Some printing to stderr, just to make the progress visual in the terminal
                if (depth == 4) {
                    cerr << "|";
                }

                // If the left child is already at least as high as the upper bound, don't bother recursing into the right child
                if (left_misclassified >= upper_bound)
                    continue;

                // Update the path (i + 501 will mean "all instances have feature i set to 1"),
                // and then recurse into the right child
                // In the end, reset the path
                // Note that the new upper bound is set to (upper_bound - left_misclassified), to take the misclassification of the left side into account as well
                path[depth - 2] = i + 501;
                int misclassified = left_misclassified + calculate_smallest_misclassification(right, depth - 1, path, upper_bound - left_misclassified);
                path[depth - 2] = 0;
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
                if (best > misclassified) {
                    // If so, update best and recompute the upper bound
                    best = misclassified;
                    upper_bound = min(upper_bound, best);

                    // If we have no misclassifications, we cannot improve any more, and can stop checking splits
                    if (best == 0) {
                        break;
                    }
                }
            }
        }

        // Cache the calculated value
        mem.insert({key, best});
    }

    // Return the cached value
    
    return mem[key];
}

int main() {
    // Read input, parse instances
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

    // Calculate the smallest misclassification starting from the root
    int path[3] = {0, 0, 0};
    int misclassification = calculate_smallest_misclassification(insts, depth, path, INF);

    // Print answer
    //cout << depth << endl;
    cout << misclassification << endl;
    cout.flush();
}
