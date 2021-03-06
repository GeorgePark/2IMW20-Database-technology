//
// Created by Nikolay Yakovets on 2018-02-02.
//

#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H


#include <memory>
#include <cmath>
#include "SimpleGraph.h"
#include "RPQTree.h"
#include "Evaluator.h"
#include "Graph.h"
#include <list>
#include <map>
#include <set>

struct Results {
    std::map<uint32_t, std::vector<uint32_t>> result;

    void removeDuplicates () {
        // Dont forget the &
        for (auto &item : result) {
            std::sort(item.second.begin(), item.second.end());
            // item.second now holds {1 2 3 4 5 6 7 x x x x x x}, where 'x' is indeterminate
            item.second.erase(std::unique(item.second.begin(), item.second.end()), item.second.end());
        }
    }
};

class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;

public:

    std::map<std::string, cardStat> cache;
    std::map<std::string, std::shared_ptr<Results>> intermediateCache;

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);

    ~SimpleEvaluator() = default;

    void prepare() override;

    cardStat evaluate(RPQTree *query) override;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<Results> evaluate_aux(RPQTree *q);

    static std::shared_ptr<Results> project(uint32_t label, bool inverse, std::shared_ptr<SimpleGraph> &g);

    static std::shared_ptr<Results> join(std::shared_ptr<Results> &left, std::shared_ptr<Results> &right);

    cardStat computeStats(std::shared_ptr<Results> &g);

    std::vector<RPQTree *> leaves(RPQTree *query);

    std::vector<RPQTree *> gen_combinations(std::vector<RPQTree *> query);
};


#endif //QS_SIMPLEEVALUATOR_H
