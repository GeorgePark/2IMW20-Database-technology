//
// Created by Nikolay Yakovets on 2018-01-31.
//

#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n) {
    setNoVertices(n);
}

uint32_t SimpleGraph::getNoVertices() const {
    return V;
}

void SimpleGraph::setNoVertices(uint32_t n) {
    V = n;
}

uint32_t SimpleGraph::getNoEdges() const {
    uint32_t sum = 0;
    for (const auto &l : edgeadj)
        sum += l.size();
    return sum;
}

// sort on the second item in the pair, then on the cardStats (ascending order)
bool sortPairs(const std::pair<uint32_t, uint32_t> &a, const std::pair<uint32_t, uint32_t> &b) {
    if (a.second < b.second) return true;
    if (a.second == b.second) return a.first < b.first;
    return false;
}

uint32_t SimpleGraph::getNoLabels() const {
    return L;
}

void SimpleGraph::setNoLabels(uint32_t noLabels) {
    L = noLabels;
    edgeadj.resize(L);
}

void SimpleGraph::addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    if (from >= V || to >= V || edgeLabel >= L)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                 "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                 std::to_string(edgeLabel) + ")");
    edgeadj[edgeLabel].emplace_back(from, to);
}

void SimpleGraph::readFromContiguousFile(const std::string &fileName) {

    std::string line;
    std::ifstream graphFile{fileName};
    std::regex headerPat(R"((\d+),(\d+),(\d+))"); // noNodes,noEdges,noLabels

    // parse the header (1st line)
    std::getline(graphFile, line);
    std::smatch matches;
    if (std::regex_search(line, matches, headerPat)) {
        uint32_t noNodes = (uint32_t) std::stoul(matches[1]);
        uint32_t noLabels = (uint32_t) std::stoul(matches[3]);

        setNoVertices(noNodes);
        setNoLabels(noLabels);
    } else {
        throw std::runtime_error(std::string("Invalid graph header!"));
    }
    std::string str;
    while (std::getline(graphFile, str))
    {
        std::stringstream testStream(str);
        std::string value;
        std::getline(testStream, value, ' ');
        uint32_t subject = (uint32_t)atoi(value.c_str());
        std::getline(testStream, value, ' ');
        uint32_t predicate = (uint32_t)atoi(value.c_str());
        std::getline(testStream, value, ' ');
        uint32_t object = (uint32_t)atoi(value.c_str());

        addEdge(subject, object, predicate);
    }
    graphFile.close();

    for (auto &edgeLabel : edgeadj) {
        std::sort(edgeLabel.begin(), edgeLabel.end(), sortPairs);
        // edgeLabel now holds {1 2 3 4 5 6 7 x x x x x x}, where 'x' is indeterminate
        edgeLabel.erase(std::unique(edgeLabel.begin(), edgeLabel.end()), edgeLabel.end());
    }
}