//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include <list>
#include <map>
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

    for(int noLabels = 0; noLabels < graph -> getNoLabels(); noLabels++) {
        cardStat test {0,0,0};
        est_result[noLabels] = test;
    }

    std::list<uint32_t > hasLabel;

    for(int source = 0; source < graph->getNoVertices(); source++) {
        for (auto labelTarget : graph->adj[source]) {

            auto label = labelTarget.first;

            est_result[label].noPaths ++;
            hasLabel.push_back(label);
        }
        hasLabel.unique();
        for(int label : hasLabel){
            est_result[label].noOut++;
        }
        hasLabel.clear();

        for (auto labelTarget : graph->reverse_adj[source]) {

            auto label = labelTarget.first;

            hasLabel.push_back(label);
        }
        hasLabel.unique();
        for(int label : hasLabel){
            est_result[label].noIn ++;
        }
        hasLabel.clear();
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

        //float result = leftGraph.noPaths * (leftGraph.noIn * (float(rightGraph.noOut) / (float)(graph->getNoVertices())) * ((float)(rightGraph.noPaths) / (float)(rightGraph.noOut)));

        //float result1 = (float)(leftGraph.noPaths * rightGraph.noPaths) / (float)(leftGraph.noOut);
        //float result2 = (float)(leftGraph.noPaths * rightGraph.noPaths) / (float)(rightGraph.noOut);
        //uint32_t final_result = std::min((int)(result1), (int)(result2));
        uint32_t result1 = (leftGraph.noPaths * rightGraph.noPaths) / leftGraph.noOut;
        uint32_t result2 = (leftGraph.noPaths * rightGraph.noPaths) / rightGraph.noOut;

        return cardStat {std::min(leftGraph.noOut, rightGraph.noOut), std::min(result1, result2),
                         std::min(leftGraph.noIn, rightGraph.noIn)};
    }

    return est_result[label];

}