#ifndef STORE_HPP
#define STORE_HPP

#include "disjointed_intervals.hpp"
#include <unordered_map>
#include <string>
#include <iostream>

class Invariant {
    std::unordered_map<std::string, DisjointedIntervals> variable_interval;
public:
    Invariant() = default;
    Invariant(const Invariant& other) : variable_interval(other.variable_interval) {}

    bool operator==(const Invariant& other) const {
        return variable_interval == other.variable_interval;
    }

    DisjointedIntervals& operator[](const std::string& var) {
        return variable_interval[var];
    }

    const DisjointedIntervals& operator[](const std::string& var) const {
        return variable_interval.at(var);
    }

    // For each of the variable in the two invariants, join the intervals
    Invariant join(const Invariant& other) {
        Invariant result;

         
        for(const auto& [var, interval] : variable_interval) {
            if(other.contains(var)) {
                result.variable_interval[var].add(interval);
                result.variable_interval[var].add(other.variable_interval.at(var));
            } else {
                // No need to join, just copy the interval
                result.variable_interval[var] = interval;
            }
        }
        for(const auto& [var, interval] : other.variable_interval) {
            // If the variable is not in the first store, just by adding it
            if(!contains(var)) {
                result.variable_interval[var] = interval;
            }
        }
        return result;
    }

    bool contains(const std::string& var) const {
        return variable_interval.find(var) != variable_interval.end();
    }

    void print() const {
        for(const auto& [var, interval] : variable_interval) {
            std::cout << var << " -> " << interval << std::endl;
        }
    }
};

#endif //STORE_HPP