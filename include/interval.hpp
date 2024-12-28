#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include <utility>
#include <algorithm>
#include <climits>
#include <assert.h>
#include <iostream>


class Interval {
    std::pair<int, int> interval;
    bool is_empty;
public:
    Interval(int a, int b): is_empty(false) {
        assert(a <= b);
        interval = std::make_pair(a, b);
    }
    Interval(): interval(-INT_MAX, +INT_MAX), is_empty(false) {}
    static Interval empty() {
        Interval empty_interval;
        empty_interval.is_empty = true;
        return empty_interval;
    }

    void set_empty(bool empty) {
        is_empty = empty;
    }
    int lb() const {
        return is_empty ? 0 : interval.first;
    }

    int ub() const {
        return is_empty ? 0 : interval.second;
    }

    void set_lb(int a) {
        if (!is_empty) interval.first = a;
    }

    void set_ub(int b) {
        if (!is_empty) interval.second = b;
    }

    Interval join(const Interval& other) const {
        if (is_empty) return other;
        if (other.is_empty) return *this;
        return Interval(std::min(interval.first, other.interval.first), std::max(interval.second, other.interval.second));
    }

    Interval meet(const Interval& other) const {
        if (is_empty || other.is_empty) return empty();
        int new_lb = std::max(interval.first, other.interval.first);
        int new_ub = std::min(interval.second, other.interval.second);
        if (new_lb > new_ub) return empty();
        return Interval(new_lb, new_ub);
    }

    Interval operator +(const Interval& other) const {
        if (is_empty || other.is_empty) return empty();
        return Interval(interval.first + other.interval.first, interval.second + other.interval.second);
    }

    Interval operator -(const Interval& other) const {
        if (is_empty || other.is_empty) return empty();
        return Interval(interval.first - other.interval.second, interval.second - other.interval.first);
    }

    Interval operator *(const Interval& other) const {
        if (is_empty || other.is_empty) return empty();
        int a = interval.first * other.interval.first;
        int b = interval.first * other.interval.second;
        int c = interval.second * other.interval.first;
        int d = interval.second * other.interval.second;
        return Interval(std::min({a, b, c, d}), std::max({a, b, c, d}));
    }

    Interval operator /(const Interval& other) const {
        if (is_empty || other.is_empty)
            return empty();

        if(other.interval.first == 0 && other.interval.second == 0){
            // Division by zero
            return empty();
        }


        // If there is zero in the interval, adjust the interval  
        // so to avoid division by zero including all the remaining values in the interval
        Interval other_copy = other;
        if(other_copy.ub() == 0)
            other_copy.set_ub(-1);
        if(other_copy.lb() == 0)
            other_copy.set_lb(1);
        
        int a = interval.first / other_copy.interval.first;
        int b = interval.first / other_copy.interval.second;
        int c = interval.second / other_copy.interval.first;
        int d = interval.second / other_copy.interval.second;
        return Interval(std::min({a, b, c, d}), std::max({a, b, c, d}));
    }

    Interval operator&(const Interval& other) const {
        return join(other);
    }

    Interval operator^(const Interval& other) const {
        return meet(other);
    }
    
    /**
     * Check if this interval is contained in the other interval
     */
    bool is_subset(const Interval& other) const {
        if (is_empty) return true;
        if (other.is_empty) return false;
        return interval.first >= other.interval.first && interval.second <= other.interval.second;
    }

    bool operator==(const Interval& other) const {
        return is_empty == other.is_empty && interval == other.interval;
    }

    bool operator!=(const Interval& other) const {
        return !(*this == other);
    }

    bool operator <=(const Interval& other) const {
        if (is_empty) return true;
        if (other.is_empty) return false;
        return interval.second <= other.interval.first;
    }

    bool operator <(const Interval& other) const {
        if (is_empty) return true;
        if (other.is_empty) return false;
        return interval.second < other.interval.first;
    }

    bool operator >=(const Interval& other) const {
        if (is_empty) return false;
        if (other.is_empty) return true;
        return interval.first >= other.interval.second;
    }

    bool operator >(const Interval& other) const {
        if (is_empty) return false;
        if (other.is_empty) return true;
        return interval.first > other.interval.second;
    }

    bool isEmpty() const {
        return is_empty;
    }

    void print() const {
        if (is_empty) {
            std::cout << "Empty interval" << std::endl;
        } else {
            std::cout << "[" << interval.first << ", " << interval.second << "]" << std::endl;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Interval& interval) {
        if (interval.is_empty) {
            os << "⊥";
        } else if(interval == Interval(-INT_MAX, +INT_MAX)){
            os << "⊤";
        } else {
            os << "[" << interval.interval.first << ", " << interval.interval.second << "]";
        }
        return os;
    }
};

#endif //INTERVAL_HPP