//
// Created by Nikolay Yakovets on 2018-02-01.
//

#include "SimpleGraph.h"
#include "SimpleEstimator.h"

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

    for(int source = 0; source < graph->getNoVertices(); source++) {
        for (auto labelTarget : graph->adj[source]) {

            auto label = labelTarget.first;

            first[label].noPaths ++;

        }
    }

}

cardStat SimpleEstimator::estimate(RPQTree *q) {

    // perform your estimation here

    return first[0];
}