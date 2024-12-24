#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "store.hpp"
#include "interval.hpp"
#include <functional>

namespace semantics {

    using Command = std::function<void(Store&)>;
    struct CommandInterpreter {
        static void evaluate(Command com, Store& store){
            com(store);
        }
    };

    struct Expression {
        static Interval evaluate(std::string variable, const Store& store){
            return store[variable];
        }

        static Interval evaluate(int constant, const Store& store){
            return Interval(constant, constant);
        }
    };
    
    
}


#endif