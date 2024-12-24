#include "abstract_interpreter.hpp"


void AbstractInterpreter::init_equations(const ASTNode& node) {

    if(node.type == NodeType::DECLARATION){
        std::cout << "[Log] Declaration found." << std::endl;
        ASTNode child = node.children[0];
        assert(child.type == NodeType::VARIABLE);
        std::cout << "[Log] Variable found." << std::endl;
        std::string var = std::get<std::string>(child.value);
        // Save the node of this control point
        cp_nodes.push_back(std::make_unique<ASTNode>(child));
        // Create an assignment command 
        commands.push_back([var](Store& store){
            store[var] = Interval(); // Initialize interval to top element
        });
    }
    
    if(node.type == NodeType::ASSIGNMENT){
        std::cout << "[Log] Assignment found." << std::endl;

        // Name of the variable we are assigning to the value 
        std::string var = std::get<std::string>(node.children[0].value);

        // Save the node of this control point
        cp_nodes.push_back(std::make_unique<ASTNode>(node));

        ASTNode second_child = node.children[1];
        if(second_child.type == NodeType::INTEGER){
            std::cout << "\t[Log] Assigning an integer." << std::endl;
            int value = std::get<int>(second_child.value);
            std::cout << "\t[Log] Value: " << value << std::endl;
            // Create an assignment command 
            commands.push_back([var, value](Store& store){
                store[var] = semantics::Expression::evaluate(value, store); // Evaluate the expression of a constant
            });
        }
        else if(second_child.type == NodeType::VARIABLE){
            std::cout << "\t[Log] Assigning a variable." << std::endl;
            std::string var_name = std::get<std::string>(second_child.value);
            std::cout << "\t[Log] Value: " << var_name << std::endl;

            // Create an assignment command 
            commands.push_back([var, var_name](Store& store){
                store[var] = semantics::Expression::evaluate(var_name, store);  
                
            });
        }
    }

    for(const auto& child : node.children){
        init_equations(child);
    }
}

/**
 * Applies the commands to the store and checks if the fixed point is reached.
 * @return True if the fixed point is reached, false otherwise
 */
bool AbstractInterpreter::solve_step() {

    std::cout << "[Log] Solving step." << std::endl;

    // For each command, execute it
    Store old_store(stores.back());
    for(auto& command : commands){
        semantics::CommandInterpreter::evaluate(command, stores.back());
    }

    // Check if the fixed point is reached
    if(old_store == stores.back()){
        return true;
    }else{
        return false;
    }
    
}

void AbstractInterpreter::solve_equations() {

    std::cout << "[Log] Number of commands: " << commands.size() << std::endl;
    // Solve the equations until the fixed point is reached
    bool fixed_point_reached = false;
    do{
        fixed_point_reached = solve_step();
    }while(!fixed_point_reached);
    
    std::cout << "[Log] Fixed point reached." << std::endl;
}