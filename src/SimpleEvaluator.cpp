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
        for (uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->reverse_adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, label);
            }
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph>
SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

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
    return out;
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
    // Check if a query is at least 3 long first
    if (query->isConcat()) {
        if (query->left->isConcat() || query->right->isConcat()) {
            //TODO: Create a 'better' query using the estimator to find an order for joining
            std::cout << "Hello\n";
            for (auto leaf : leaves(query)) {
                std::cout << leaf;
            }
            std::cout << "\n";
        }
    }
    auto res = evaluate_aux(query);
    return SimpleEvaluator::computeStats(res);
}

// Function to get a list of all the leaves of a query
std::list<RPQTree *> SimpleEvaluator::leaves(RPQTree *query) {
    if (query->isLeaf()) {
        std::list<RPQTree *> leaf;
        leaf.emplace_back(query);
        return leaf;
    }
    std::list<RPQTree *> leafs;

    // Get the leaves from left to right and add them to the list.
    leaves(query->left);
    /*leafs.insert(position, List.begin(), List.end());*/

    leaves(query->right);
    /*leafs.insert(position, List.begin(), List.end());*/

    return leafs;
}

/*TODO: Implement the following:
 * labels = ["0+", "1+", "2+", "2+", "3+", "4+"]

def generateCombinations(sublist):
    results = []

    if len(sublist) == 1: return sublist

    for i in range(1, len(sublist)):
        a = generateCombinations(sublist[:i])
        b = generateCombinations(sublist[i:])

        for r in a:
            for s in b:
                results += ["(" + r + "/" + s + ")"]

    return results

results = generateCombinations(labels)
 */