//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include <list>
#include <map>

std::regex directLabel (R"((\d+)\+)");
std::regex inverseLabel (R"((\d+)\-)");

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {

    // do your prep here
    std::list<uint32_t> uniqueNodesForLabel;

    for(int noLabels = 0; noLabels < graph->getNoLabels(); noLabels++) {
        est_result[noLabels] = cardStat {0, 0, 0};
        est_result[noLabels].noPaths = graph->edgeadj[noLabels].size();
        if (est_result[noLabels].noPaths != 0) {
            uint32_t helper = (uint32_t) (((float) (est_result[noLabels].noPaths) / (float) (graph->getNoEdges())) *
                                          graph->getNoVertices());
            est_result[noLabels].noOut = helper;
            est_result[noLabels].noIn = helper;
        }
    }

    for (int noLabels = 0; noLabels < graph->getNoLabels(); noLabels++) {

    }

}

cardStat SimpleEstimator::estimate(RPQTree *q) {

    // perform your estimation here

    // project out the label in the AST

    std::smatch matches;

    uint32_t label;
    bool inverse = false;

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

        if (!inverse) {
            leftGraph = SimpleEstimator::estimate(q->left);
            rightGraph = SimpleEstimator::estimate(q->right);
        }
        else {
            leftGraph = SimpleEstimator::estimate(q->right);
            rightGraph = SimpleEstimator::estimate(q->left);
        }
        uint32_t result1 = (leftGraph.noPaths * rightGraph.noPaths) / leftGraph.noOut;
        uint32_t result2 = (leftGraph.noPaths * rightGraph.noPaths) / rightGraph.noOut;

        return cardStat {std::min(leftGraph.noOut, rightGraph.noOut), std::min(result1, result2),
                         std::min(leftGraph.noIn, rightGraph.noIn)};
    }

    return est_result[label];
}