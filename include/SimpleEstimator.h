//
// Created by Nikolay Yakovets on 2018-02-01.
//

#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"
#include <list>
#include <map>

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;
    std::map<uint32_t, cardStat> est_result;
    std::map<uint32_t , std::list<uint32_t>> hasLabel;
    std::map<uint32_t , std::list<uint32_t>> hasLabelReverse;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(RPQTree *q) override ;

};


#endif //QS_SIMPLEESTIMATOR_H
