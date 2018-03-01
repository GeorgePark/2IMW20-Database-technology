//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include <list>
#include <map>
#include "cmath"

std::map<uint32_t , cardStat> first;

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // do your prep here

    for(int noLabels = 0; noLabels < graph -> getNoLabels(); noLabels++) {
        cardStat test {0,0,0};
        first[noLabels] = test;
    }

    std::list<uint32_t > hasLabel;

    for(int source = 0; source < graph->getNoVertices(); source++) {
        for (auto labelTarget : graph->adj[source]) {

            auto label = labelTarget.first;

            first[label].noPaths ++;
            hasLabel.push_back(label);
        }
        hasLabel.unique();
        for(int label : hasLabel){
            first[label].noOut++;
        }
        hasLabel.clear();

        for (auto labelTarget : graph->reverse_adj[source]) {

            auto label = labelTarget.first;

            hasLabel.push_back(label);
        }
        hasLabel.unique();
        for(int label : hasLabel){
            first[label].noIn ++;
        }
        hasLabel.clear();
    }

}

cardStat SimpleEstimator::estimate(RPQTree *q) {

    // perform your estimation here

    // project out the label in the AST
    std::regex directLabel (R"((\d+)\+)");
    std::regex inverseLabel (R"((\d+)\-)");

    std::smatch matches;

    uint32_t label;
    bool inverse;

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
        auto leftGraph = SimpleEstimator::estimate(q->left);
        auto rightGraph = SimpleEstimator::estimate(q->right);

        return cardStat {static_cast<uint32_t>(std::ceil((leftGraph.noOut + rightGraph.noOut) / 2)), leftGraph.noPaths + rightGraph.noPaths,
                         static_cast<uint32_t>(std::ceil((leftGraph.noIn + rightGraph.noIn) / 2))};
    }
    // std::min(leftGraph.noIn, rightGraph.noIn)
    // static_cast<uint32_t>(std::ceil((leftGraph.noOut + rightGraph.noOut) / 2))

    return first[label];
}