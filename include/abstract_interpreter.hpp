#ifndef ABSTRACT_INTERPRETER_HPP
#define ABSTRACT_INTERPRETER_HPP

#include "store.hpp"
#include "interval.hpp"
#include "ast.hpp"
#include "semantics.hpp"

#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <assert.h>
class AbstractInterpreter {
private:
    // For each control point, create a command that will be executed and will modify the store
    std::vector<std::unique_ptr<semantics::Command>> commands;
    // For each control point, store the invariants
    std::vector<Store> stores;
    // For each control point, store the node
    std::vector<std::unique_ptr<ASTNode>> cp_nodes;
    
    /**
     * Solve the equational system to find the invariants
     */
    bool solve_step();


public:

    AbstractInterpreter() {
        // Push one empty store for invariant 0
        stores.push_back(Store());
    }
    /**
     * From the AST, create the equational system that we will solve to find the invariants
     */
    void init_equations(const ASTNode& node);

    /**
     * Solve the equational system to find the invariants, calling solve_step until the fixed point is reached
     */
    void solve_equations();

        /**
     * Evaluate the abstract syntax tree, searcing if there are any invariant violations
     * @param node The root node of the AST
     */
    void eval(const ASTNode& node);


    void print_store() const {
        for(const auto& s : stores){
            s.print();
        }
    }
    
};



#endif //ABSTRACT_INTERPRETER_HPP