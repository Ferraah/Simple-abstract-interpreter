#ifndef STORE_HPP
#define STORE_HPP

#include "disjointed_intervals.hpp"
#include <unordered_map>
#include <string>
#include <iostream>

class Invariant {
    std::unordered_map<std::string, DisjointedIntervals> variable_interval;
    bool is_zero_invariant = false;
public:
    Invariant() = default;
    Invariant(const Invariant& other) : variable_interval(other.variable_interval) {}

    void set_zero_invariant(bool flag) {
        is_zero_invariant = flag;
    }

    bool get_is_zero_invariant() {
        return is_zero_invariant;
    }

    bool operator==(const Invariant& other) const {
        return variable_interval == other.variable_interval;
    }

    DisjointedIntervals& operator[](const std::string& var) {
        return variable_interval[var];
    }

    const DisjointedIntervals& operator[](const std::string& var) const {
        return variable_interval.at(var);
    }

    size_t size() const {
        return variable_interval.size();
    }

    // For each of the variable in the two invariants, join the intervals
    Invariant join(const Invariant& other) {
        Invariant result;

        // For each variable in this invariant, join thie intervals with the other invariant
        for(const auto& [var, interval] : variable_interval) {
            // If the other contains this variable, we join the two intervals

            if(other.contains(var)) {
                result[var] = result[var].join(interval);
                result[var] = result[var].join(other[var]);

            } else 
            // If it's new, we just copy the interval
            {
                result[var] = interval;
            }
        }

        for(const auto& [var, interval] : other.variable_interval) {
            // Adding the remaining variables from the other invariant not present in this one
            if(!contains(var)) {
                result[var] = interval;
            }
        }
        return result;
    }

    bool contains(const std::string& var) const {
        return variable_interval.find(var) != variable_interval.end();
    }

    void print() const {
        if(variable_interval.empty()) {
            std::cout << "No assignments." << std::endl;
        }else{
            for(const auto& [var, interval] : variable_interval) {
                std::cout << var << " -> " << interval << std::endl;
            }
        }
    }
};

#endif //STORE_HPP