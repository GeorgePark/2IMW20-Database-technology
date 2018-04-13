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

cardStat SimpleEvaluator::computeStats(std::shared_ptr<Results> &g) {

    g->removeDuplicates();

    cardStat stats{};

    std::vector<uint8_t> noIn(graph->getNoVertices());

    for (auto item : g->result) {
        if (!item.second.empty()) {
            stats.noOut++;
            stats.noPaths += item.second.size();
            for (auto target : item.second) {
                noIn[target] = 1;
            }
        }
    }

    stats.noIn = std::count(noIn.begin(), noIn.end(), 1);

    return stats;
}

std::shared_ptr<Results>
SimpleEvaluator::project(uint32_t projectLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<Results>(Results());
    if (!inverse) {
        for (auto labelTarget : in->edgeadj[projectLabel]) {

            auto source = labelTarget.first;
            auto target = labelTarget.second;

            out->result[source].push_back(target);
        }
    } else {
        for (auto labelTarget : in->edgeadj[projectLabel]) {

            auto source = labelTarget.first;
            auto target = labelTarget.second;

            out->result[target].push_back(source);
        }
    }

    return out;
}

std::shared_ptr<Results>
SimpleEvaluator::join(std::shared_ptr<Results> &left, std::shared_ptr<Results> &right) {

    auto out = std::make_shared<Results>(Results());

    // For all left results
    for (const auto &leftResult : left->result) {
        // Get the vector of targets
        for (auto rightSources : leftResult.second) {
            // Append the vector of rightTargets with source leftTarget (leftResult.second) to the new results
            // with key leftSource (leftResult.first)
            out->result[leftResult.first].insert(out->result[leftResult.first].end(),
                                                 right->result[rightSources].begin(),
                                                 right->result[rightSources].end());
        }
    }
    out->removeDuplicates();
    return out;
}

std::shared_ptr<Results> SimpleEvaluator::evaluate_aux(RPQTree *q) {

    std::string query;
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

        for (auto item : leaves(q)) {
            query += item->data;
        }

        if (intermediateCache.count(query) > 0) {
            return intermediateCache[query];
        } else {
            // join left with right
            intermediateCache[query] = SimpleEvaluator::project(label, inverse, graph);
            return intermediateCache[query];
        }

        return SimpleEvaluator::project(label, inverse, graph);
    }

    if (q->isConcat()) {

        // evaluate the children
        auto leftGraph = SimpleEvaluator::evaluate_aux(q->left);
        auto rightGraph = SimpleEvaluator::evaluate_aux(q->right);

        for (auto item : leaves(q)) {
            query += item->data;
        }

        if (intermediateCache.count(query) > 0) {
            return intermediateCache[query];
        } else {
            // join left with right
            intermediateCache[query] = SimpleEvaluator::join(leftGraph, rightGraph);
            return intermediateCache[query];
        }
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