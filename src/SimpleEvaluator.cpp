//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if (est != nullptr) est->prepare();

    // prepare other things here.., if necessary

}

// project out the label in the AST
std::regex directLabelEval(R"((\d+)\+)");
std::regex inverseLabelEval(R"((\d+)\-)");

cardStat SimpleEvaluator::computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats{};

    for (int source = 0; source < g->getNoVertices(); source++) {
        if (!g->adj[source].empty()) stats.noOut++;
        if (!g->reverse_adj[source].empty()) stats.noIn++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    return stats;
}

std::shared_ptr<SimpleGraph>
SimpleEvaluator::project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if (!inverse) {
        // going forward
        for (uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    } else {
        // going backward
        for (uint32_t target = 0; target < in->getNoVertices(); target++) {
            for (auto labelTarget : in->adj[target]) {

                auto label = labelTarget.first;
                auto source = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph>
SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    if (seen_graphs.count(std::make_pair(left, right))) {
        return seen_graphs[std::make_pair(left, right)];
    } else {
        auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
        out->setNoLabels(1);

        for (uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
            for (auto labelTarget : left->adj[leftSource]) {

                int leftTarget = labelTarget.second;
                // try to join the left target with right source
                for (auto rightLabelTarget : right->adj[leftTarget]) {

                    auto rightTarget = rightLabelTarget.second;
                    out->addEdge(leftSource, rightTarget, 0);

                }
            }
        }
        seen_graphs[std::make_pair(left, right)] = out;
        return out;
    }
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluate_aux(RPQTree *q) {

    // evaluate according to the AST bottom-up
    if (q->isLeaf()) {

        std::smatch matches;

        uint32_t label;
        bool inverse;

        if (std::regex_search(q->data, matches, directLabelEval)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = false;
        } else if (std::regex_search(q->data, matches, inverseLabelEval)) {
            label = (uint32_t) std::stoul(matches[1]);
            inverse = true;
        } else {
            std::cerr << "Label parsing failed!" << std::endl;
            return nullptr;
        }

        return SimpleEvaluator::project(label, inverse, graph);
    }

    if (q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEvaluator::evaluate_aux(q->left);
        auto rightGraph = SimpleEvaluator::evaluate_aux(q->right);

        // join left with right
        return SimpleEvaluator::join(leftGraph, rightGraph);

    }

    return nullptr;
}

cardStat SimpleEvaluator::evaluate(RPQTree *query) {
    uint32_t highest = UINT32_MAX;
    std::vector<RPQTree *> leafs = leaves(query);
    std::string data;
    for (auto item : leafs) {
        data += item->data;
    }
    if (cache.count(data)) {
        return cache[data];
    } else {
        RPQTree *newQuery = query;
        std::vector<RPQTree *> combinations = gen_combinations(leafs);
        for (auto item : combinations) {
            uint32_t current = est->estimate(item).noPaths;
            if (current < highest) {
                newQuery = item;
                highest = current;
            }
        }
        auto res = evaluate_aux(newQuery);
        cache[data] = SimpleEvaluator::computeStats(res);
        return cache[data];
    }
}

// Function to get a list of all the leaves of a query
std::vector<RPQTree *> SimpleEvaluator::leaves(RPQTree *query) {
    // Vector containing the leaves of a RPQTree
    std::vector<RPQTree *> leafs;
    if (query->isLeaf()) {
        leafs.push_back(new RPQTree(query->data, nullptr, nullptr));
        return leafs;
    }
    // Vector containing the return value of a previous call
    std::vector<RPQTree *> ret;

    // Get the leaves from left to right and add them to the list.
    ret = leaves(query->left);
    leafs.insert(leafs.end(), ret.begin(), ret.end());

    ret = leaves(query->right);
    leafs.insert(leafs.end(), ret.begin(), ret.end());

    return leafs;
}

std::vector<RPQTree *> SimpleEvaluator::gen_combinations(std::vector<RPQTree *> query) {
    std::vector<RPQTree *> combinations;

    // If there is only one item remaining in the vector, return it
    if (query.size() < 2) {
        return query;
    }

    for (int i = 1; i < query.size(); i++) {
        std::vector<RPQTree *> split_left(query.begin(), query.begin() + i);
        std::vector<RPQTree *> split_right(query.begin() + i, query.end());

        auto left = gen_combinations(split_left);
        auto right = gen_combinations(split_right);

        for (auto left_item : left) {
            for (auto right_item : right) {
                std::string data = "/";
                RPQTree *comb = new RPQTree(data, left_item, right_item);
                combinations.emplace_back(comb);
            }
        }
    }
    return combinations;
}