#ifndef DISJOINTED_INTERVALS_HPP
#define DISJOINTED_INTERVALS_HPP

#include <set>
#include <vector>
#include <algorithm>
#include "interval.hpp"

class DisjointedIntervals {
public:
    DisjointedIntervals() = default;
    DisjointedIntervals(const Interval& interval) {
        intervals.insert(interval);
    }
    DisjointedIntervals(const std::vector<Interval>& intervals) {
        for (const auto& interval : intervals) {
            addInterval(interval);
        }
    }
    DisjointedIntervals(const DisjointedIntervals& other) {
        intervals = other.intervals;
    }

    DisjointedIntervals(int lb, int ub) {
        intervals.insert(Interval(lb, ub));
    }
    
    DisjointedIntervals join(const DisjointedIntervals& other) {
        if(other.intervals.empty()) return *this;
        if(intervals.empty()) return other;

        return DisjointedIntervals(Interval(std::min(lb(), other.lb()), std::max(ub(), other.ub())));

    }

    int ub() const {
        if (intervals.empty()) return 0;
        return intervals.rbegin()->ub();
    }

    int lb() const {
        if (intervals.empty()) return 0;
        return intervals.begin()->lb();
    }

    
    /**
     * @brief Adds a new interval to the set of disjoint intervals.
     *
     * This function takes an interval and merges it with any overlapping intervals
     * in the set of disjoint intervals. The result is a set of disjoint intervals
     * that includes the new interval.
     *
     * @param other The interval to be added.
     */
    void addInterval(const Interval& other) {

        if(other.isEmpty()) return;

        Interval newInterval = other;

        // Pick the first interval that is greater than or equal to the new interval
        auto it = intervals.lower_bound(newInterval);

        // Check if there is an overlapping interval before the current interval
        if (it != intervals.begin() && std::prev(it)->ub() >= newInterval.lb()) {
            --it;
        }

        // For each following interval, merge it with the new interval
        while (it != intervals.end() && it->lb() <= newInterval.ub()) {
            newInterval = newInterval.join(*it);
            it = intervals.erase(it);
        }

        // Insert the merged interval
        intervals.insert(newInterval);

        // Handle adjacent intervals, i.e merge the if the boundaries overlaps
        for(auto it = intervals.begin(); it != intervals.end();){
            auto next = std::next(it);
            if(next != intervals.end() && it->ub() + 1 == next->lb()){
                Interval newInterval(it->lb(), next->ub());
                it = intervals.erase(it);
                intervals.erase(next);
                intervals.insert(newInterval);
            } else {
                ++it;
            }
        }
    }


    void filter_leq(DisjointedIntervals &other) {

        if(other.intervals.empty()) return;
        if(intervals.empty()) return;
        int other_lb = other.intervals.begin()->lb();
        int other_ub = other.intervals.begin()->ub();
        // Cut everything that is greater than the lower bound of the other set

        // Get the first interval that is greater than or equal to the lower bound of the other set
        
        auto it = intervals.lower_bound(Interval(other_lb, other_lb));
        
        if (it == intervals.end()) return;
        int old_lb = it->lb();
        if(old_lb <= other_lb){

            it = intervals.erase(it);
            intervals.insert(Interval(old_lb, other_lb));
        }
        intervals.erase(it, intervals.end());

    }

    void filter_l(DisjointedIntervals &other) {

        if(other.intervals.empty()) return;
        if(intervals.empty()) return;
        int other_lb = other.intervals.begin()->lb();
        int other_ub = other.intervals.begin()->ub();
        // Cut everything that is greater than the lower bound of the other set

        // Get the first interval that is greater than or equal to the lower bound of the other set
        
        auto it = intervals.lower_bound(Interval(other_lb, other_lb));
        
        if (it == intervals.end()) return;
        int old_lb = it->lb();
        if(old_lb < other_lb){

            it = intervals.erase(it);
            intervals.insert(Interval(old_lb, other_lb-1));
        }
        intervals.erase(it, intervals.end());

    }


    void filter_g(DisjointedIntervals &other) {

        if(other.intervals.empty()) return;
        if(intervals.empty()) return;
        int other_lb = other.intervals.begin()->lb();
        int other_ub = other.intervals.begin()->ub();
        // Cut everything that is greater than the lower bound of the other set

        // Get the first interval that is greater than or equal to the lower bound of the other set
        auto it = intervals.begin();
        while(it != intervals.end() && it->ub() < other_ub){
            ++it;
        }
        std::cout << "it: " << *it << std::endl;
        int old_ub = it->ub();
        
        if(old_ub > other_ub){

            intervals.erase(it);
            intervals.insert(Interval(other_ub+1, old_ub));
        }
        intervals.erase(intervals.begin(), it); 

    }


    void filter_geq(DisjointedIntervals &other) {

        if(other.intervals.empty()) return;
        if(intervals.empty()) return;
        int other_lb = other.intervals.begin()->lb();
        int other_ub = other.intervals.begin()->ub();
        // Cut everything that is greater than the lower bound of the other set

        // Get the first interval that is greater or equal than the lower bound of the other set
        auto it = intervals.begin();
        while(it != intervals.end() && it->ub() < other_ub){
            ++it;
        }
        int old_ub = it->ub();
        
        if(old_ub > other_ub){

            intervals.erase(it);
            intervals.insert(Interval(other_ub, old_ub));
        }
        intervals.erase(intervals.begin(), it); 

    }

    

    void filter_eq(DisjointedIntervals &other) {

        if(other.intervals.empty()) return;
        if(intervals.empty()) return;
        
        *this = meet(other);

    }

    void filter_neq(DisjointedIntervals &other) {

        if(other.intervals.empty()) return;
        if(intervals.empty()) return;
        
        remove(other);

    }


    void add(const DisjointedIntervals& other) {
        for (const auto& interval : other.intervals) {
            addInterval(interval);
        }
    }

    void add(const std::vector<Interval>& other) {
        for (const auto& interval : other) {
            addInterval(interval);
        }
    }
    void remove(const DisjointedIntervals& other) {
        for (const auto& interval : other.intervals) {
            removeInterval(interval);
        }
    }
    
    DisjointedIntervals operator+(const DisjointedIntervals& other) const {
        if(other.intervals.empty()) return *this;
        if(intervals.empty()) return other;

        DisjointedIntervals a, b;
        int other_lb = other.intervals.begin()->lb();
        int other_ub = other.intervals.begin()->ub();

        a = b = *this;

        auto a_shifted = a.shiftIntervals(other_ub);
        auto b_shifted = b.shiftIntervals(other_lb);

        a_shifted = a_shifted.join(b_shifted);
        return a_shifted;
    }

    DisjointedIntervals operator-(const DisjointedIntervals& other) const {
        if (other.intervals.empty()) return *this;
        if(intervals.empty()) return *this;

        DisjointedIntervals a, b;
        int other_lb = other.intervals.begin()->lb();
        int other_ub = other.intervals.begin()->ub();

        a = b = *this;

        auto a_shifted = a.shiftIntervals(-other_lb);
        auto b_shifted = b.shiftIntervals(-other_ub);
        a_shifted =  a_shifted.join(b_shifted);
        return a_shifted;
    }

    DisjointedIntervals operator*(const DisjointedIntervals& other) const {
        if(other.intervals.empty() || intervals.empty()) return DisjointedIntervals();
        DisjointedIntervals res;
        for (const auto& interval : intervals) {
            for (const auto& otherInterval : other.intervals) {
            Interval newInterval = interval * otherInterval;
                if (!newInterval.isEmpty()) {
                    res.addInterval(newInterval);
                }
            }
        }
        return res;
    }

    DisjointedIntervals operator/(const DisjointedIntervals& other) const {
        if(other.intervals.empty() || intervals.empty()) return DisjointedIntervals();
        DisjointedIntervals res;
        for (const auto& interval : intervals) {
            for (const auto& otherInterval : other.intervals) {
                Interval newInterval = interval / otherInterval;
                if (!newInterval.isEmpty()) {
                    res.addInterval(newInterval);
                }
            }
        }
        return res;
    }

    DisjointedIntervals shiftIntervals(int shift) const {
        DisjointedIntervals res;
        for (const auto& interval : intervals) {
            res.addInterval(interval + Interval(shift, shift));
        }
        return res;
    }
    void removeInterval(const Interval& other) {

        Interval newInterval = other;
        int start = newInterval.lb();
        int end = newInterval.ub();

        // Pick the first interval that is greater than or equal to the new interval
        // to start form there.
        auto it = intervals.lower_bound(newInterval);
        
        if (it != intervals.begin() && std::prev(it)->ub() >= start) {
            --it;
        }

        // While there are intervals that overlap with the new interval ... 
        while (it != intervals.end() && it->lb() <= end) {
            std::vector<Interval> toAdd;
            
            // Get the intersection between the current interval and the new interval
            Interval a = it->meet(newInterval);

            // Add the two new intervals that are created by the intersection (if they exist)
            if (it->lb() < start) {
                toAdd.push_back(Interval(it->lb(), start - 1));
            }
            if (it->ub() > end) {
                toAdd.push_back(Interval(end + 1, it->ub()));
            }

            // The old interval is removed
            it = intervals.erase(it);

            // The new ones are added, making a "hole"
            for (const auto& interval : toAdd) {
                intervals.insert(interval);
            }
        }
    }


    
    DisjointedIntervals meet(const DisjointedIntervals& other) {
        std::vector<Interval> newIntervals;
        for (const auto& interval : intervals) {
            for (const auto& otherInterval : other.intervals) {
                Interval newInterval = interval.meet(otherInterval);
                if (!newInterval.isEmpty()) {
                    newIntervals.push_back(newInterval);
                }
            }
        }
        DisjointedIntervals res;
        for (const auto& interval : newIntervals) {
            res.addInterval(interval);
        }
        return res;
    }


    bool operator==(const DisjointedIntervals& other) const {
        return intervals == other.intervals;
    }
    bool operator!=(const DisjointedIntervals& other) const {
        return intervals != other.intervals;
    }
    bool operator<(const DisjointedIntervals& other) const {
        
        return intervals.begin()->ub() < other.intervals.begin()->lb();
    }

    bool operator>(const DisjointedIntervals& other) const {
        return intervals.begin()->lb() > other.intervals.begin()->ub();
    }
    bool operator<=(const DisjointedIntervals& other) const {
        return intervals.begin()->ub() <= other.intervals.begin()->lb();
    }
    bool operator>=(const DisjointedIntervals& other) const {
        return intervals.begin()->lb() >= other.intervals.begin()->ub();
    }
    
    

    bool contains(int point) const {
        Interval pointInterval(point, point);
        auto it = intervals.upper_bound(pointInterval);
        if (it == intervals.begin()) {
            return false;
        }
        --it;
        return it->lb() <= point && point <= it->ub();
    }

    bool contains(const Interval& other) const {
        for (const auto& interval : intervals) {
            if (other.is_subset(interval)) {
                return true;
            }
        }
        return false;
    }

    size_t size() const {
        return intervals.size();
    }
    
    std::vector<Interval> getIntervals() const {
        return std::vector<Interval>(intervals.begin(), intervals.end());
    }

    void print() const {
        if(intervals.empty()){
            std::cout << "⊥*" << std::endl;
            return;
        }else if(size() == 1 && contains(Interval(-INT_MAX, +INT_MAX))){
            std::cout << "⊤*" << std::endl;
        }else{
            for (auto it = intervals.begin(); it != intervals.end(); ++it) {
                if (it != intervals.begin()) {
                    std::cout << " U ";
                }
                std::cout << *it;
            }
        }

    }

    friend std::ostream& operator<<(std::ostream& os, const DisjointedIntervals& disjointedIntervals) {
        if(disjointedIntervals.intervals.empty()){
            os << "⊥*" << std::endl;
            return os;
        }
        else if(disjointedIntervals.size() == 1 && disjointedIntervals.contains(Interval(-INT_MAX, +INT_MAX))){
            os << "⊤*" << std::endl;
        }else{
            for (auto it = disjointedIntervals.intervals.begin(); it != disjointedIntervals.intervals.end(); ++it) {
                if (it != disjointedIntervals.intervals.begin()) {
                    os << " U ";
                }
                os << *it;
            }
        }
        return os;
    }
private:
    struct IntervalCompare {
        bool operator()(const Interval& a, const Interval& b) const {
            return a.ub() < b.lb();
        }
    };

    std::set<Interval, IntervalCompare> intervals;
};

#endif