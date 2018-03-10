//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include <list>
#include <set>
#include "cmath"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

// project out the label in the AST
std::regex directLabel (R"((\d+)\+)");
std::regex inverseLabel (R"((\d+)\-)");



void SimpleEstimator::prepare() {

    // do your prep here

    for(int noLabels = 0; noLabels < graph->getNoLabels(); noLabels++) {
        est_result[noLabels] = cardStat {0, 0, 0};
    }

    for (int source = 0; source < graph->getNoVertices(); source++) {
        for (auto labelSource : graph->adj[source]) {
            est_result[labelSource.first].noPaths++;
            hasLabel[labelSource.first].insert(labelSource.second);
        }
    }
    for (int noLabels = 0; noLabels < graph->getNoLabels(); noLabels++) {
        est_result[noLabels].noIn = hasLabel[noLabels].size();
    }
    hasLabel.clear();
    for (int source = 0; source < graph->getNoVertices(); source++) {
        for (auto labelSource : graph->reverse_adj[source]) {
            hasLabel[labelSource.first].insert(labelSource.second);
        }
    }
    for (int noLabels = 0; noLabels < graph->getNoLabels(); noLabels++) {
        est_result[noLabels].noOut = hasLabel[noLabels].size();
    }

}

cardStat SimpleEstimator::estimate(RPQTree *q) {

    // perform your estimation here

    std::smatch matches;
    bool inverse = false;

    uint32_t label;

    if (q->isLeaf()) {
        if (std::regex_search(q->data, matches, directLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = false;
        } else if (std::regex_search(q->data, matches, inverseLabel)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
            return {0, 0, 0};
        }
    }

    if (q->isConcat()) {
        // evaluate the children
        cardStat leftGraph;
        cardStat rightGraph;

        leftGraph = SimpleEstimator::estimate(q->left);
        rightGraph = SimpleEstimator::estimate(q->right);

        uint32_t result1 = (leftGraph.noPaths * rightGraph.noPaths) / leftGraph.noOut;
        uint32_t result2 = (leftGraph.noPaths * rightGraph.noPaths) / rightGraph.noOut;

        return cardStat {(leftGraph.noOut + rightGraph.noOut) / 2, std::min(result1, result2),
                        (leftGraph.noIn + rightGraph.noIn) / 2};
    }

    if (inverse) {
        return {est_result[label].noIn, est_result[label].noPaths, est_result[label].noOut};
    } else{
        return est_result[label];
    }
}