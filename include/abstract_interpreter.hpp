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

// Invariants associated at every control point, ordered. 
using InvariantsSystem = std::vector<Invariant>;

// Pointers to the Command/Join classes related to the invariants. Not strictly in order.
using SystemSolverComponents = std::vector<std::shared_ptr<semantics::ControlPointAction>>;



class AbstractInterpreter {
private:

    // Commands/Joins which update the invariants at every time step.
    SystemSolverComponents solver_components;

    // List of the invariants, which represents the environment at each control point
    // and are update at each time steps through the solver_components.
    InvariantsSystem invariants;

    // During a command, there could be a warning such as zero division or assertion not satisifed.
    // Maps a AST node to a string specifing the warning.
    std::unordered_map<size_t, std::string> warnings_list; 

    /**
     * Solve the equational system to find the invariants
     */
    bool solve_step();

    /**
     * From a AST node containing an arithmetic operation, it recursively build a BinaryOperation object.
     * @param node The operation node
     * @param add_warning_to_list The function which will add a specified string to the warning list if there is any
     */
    std::shared_ptr<semantics::BinaryOp> create_binop(const ASTNode& node, std::function<void(std::string)> add_warning_to_list);

public:

    /**
     * Recursively build the equational system of solver_components, given the AST node.
     * @param node current node. User should call it from root.
     */
    void init_equations(const ASTNode& node);

    /**
     * Solve the equational system by applying the solver_components iteratively until the fixed point is reached.
     * When calling solver_components, warnings are added to the list.
     */
    void solve_equations();

    /**
     * Print the environment at every location point.
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
    
    /**
     * Print the warnings which have been previously added during the solving phase.
     */
    void print_warnings() const {
        std::cout << "--------- WARNINGS/ERRORS RECAP ---------" << std::endl;
        for(const auto& [node_id, warning] : warnings_list) {
            std::cout << "AST node id: " << node_id << ": ";
            std::cout << warning << std::endl;
        }
        std::cout << "-----------------------------------------" << std::endl;
    }


};



#endif //ABSTRACT_INTERPRETER_HPP