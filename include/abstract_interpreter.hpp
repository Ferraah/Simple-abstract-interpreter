#ifndef ABSTRACT_INTERPRETER_HPP
#define ABSTRACT_INTERPRETER_HPP

#include "invariant.hpp"
#include "interval.hpp"
#include "ast.hpp"
#include "semantics.hpp"

#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <assert.h>

using InvariantsSystem = std::vector<Invariant>;
using SystemSolverComponents = std::vector<std::unique_ptr<semantics::ControlPointAction>>;

class AbstractInterpreter {
private:
    // Components of the function which will update each invariant in the equational system
    SystemSolverComponents commands;
    // Number of invariants counted in the ATS
    size_t invariants_count = 0;
    // List of the invariants, which represents the environment at each control point
    InvariantsSystem invariants;
    // Invariant warnings, describing the control point and the error
    std::unordered_map<size_t, std::string> warnings_list; 

    /**
     * Solve the equational system to find the invariants
     */
    bool solve_step();


    std::unique_ptr<semantics::BinaryOp> create_binop(const ASTNode& node, std::function<void(std::string)> add_warning_to_list);

public:

    /**
     * From the AST, create the equational system that we will solve to find the invariants
     */
    void init_equations(const ASTNode& node);

    /**
     * Solve the equational system to find the invariants, calling solve_step until the fixed point is reached.
     * Evaluate the the correctness with respect to the post conditions and illegal operations. 
     */
    void solve_equations();

    /**
     * Print the store
     */
    void print_invariants() const {
        size_t cp_id = 0;
        for(const auto& i : invariants){
            std::cout << "Control point " << cp_id << std::endl;
            i.print();
            std::cout << std::endl;
            cp_id++;
        }
    }
    

    void print_warnings() const {
        std::cout << "--------- WARNINGS/ERRORS RECAP ---------" << std::endl;
        for(const auto& [node_id, warning] : warnings_list) {
            std::cout << "Control point " << node_id << ": ";
            std::cout << warning << std::endl;
        }
        std::cout << "-----------------------------------------" << std::endl;
    }

    size_t last_inv_id() {
        if(invariants_count == 0) return 0;
        return invariants_count - 1;
    }

};



#endif //ABSTRACT_INTERPRETER_HPP