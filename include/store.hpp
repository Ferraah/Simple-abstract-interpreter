#ifndef STORE_HPP
#define STORE_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include "interval.hpp"

class Store {
    std::unordered_map<std::string, Interval> invariants;
public:
    Store() = default;

    bool operator==(const Store& other) const {
        return invariants == other.invariants;
    }

    Interval& operator[](const std::string& var) {
        return invariants[var];
    }

    const Interval& operator[](const std::string& var) const {
        return invariants.at(var);
    }
    


    bool contains(const std::string& var) const {
        return invariants.find(var) != invariants.end();
    }

    void print() const {
        for(const auto& [var, interval] : invariants) {
            std::cout << var << " -> [" << interval.lb() << ", " << interval.ub() << "]" << std::endl;
        }
    }
};

#endif //STORE_HPP