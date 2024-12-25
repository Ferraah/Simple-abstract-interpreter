#include "abstract_interpreter.hpp"


std::unique_ptr<semantics::BinaryOp> AbstractInterpreter::create_binop(const ASTNode& node) {
    using namespace semantics;

    // Retrieve the operation from the AST
    BinOp op = std::get<BinOp>(node.value);

    // Node children, aka the operands
    ASTNode left_child = node.children[0];
    ASTNode right_child = node.children[1];

    // Create the expressions for the operands

    std::unique_ptr<Expr> left_expr = nullptr;
    if (left_child.type == NodeType::INTEGER){
        left_expr = std::make_unique<Constant>(std::get<int>(left_child.value));
    }else if(left_child.type == NodeType::VARIABLE){
        left_expr = std::make_unique<Variable>(std::get<std::string>(left_child.value));
    }else if(left_child.type == NodeType::ARITHM_OP){
        // Recursive call to create the expression
        left_expr = create_binop(left_child);
    }

    std::unique_ptr<Expr> right_expr = nullptr;
    if (right_child.type == NodeType::INTEGER){
        right_expr = std::make_unique<Constant>(std::get<int>(right_child.value));
    }else if(right_child.type == NodeType::VARIABLE){
        right_expr = std::make_unique<Variable>(std::get<std::string>(right_child.value));
    }else if(right_child.type == NodeType::ARITHM_OP){
        // Recursive call to create the expression
        right_expr = create_binop(right_child);
    }

    assert(left_expr != nullptr);
    assert(right_expr != nullptr);
    
    std::cout << "\t\t[Log] Creating binary operation with op: "<< op << std::endl;
    return std::make_unique<BinaryOp>(op, std::move(left_expr), std::move(right_expr));
}

void AbstractInterpreter::init_equations(const ASTNode& node) {
    using namespace semantics;

    if(node.type == NodeType::DECLARATION){
        std::cout << "[Log] Declaration found." << std::endl;
        ASTNode child = node.children[0];
        assert(child.type == NodeType::VARIABLE);
        std::cout << "\t[Log] Variable found." << std::endl;
        std::string var = std::get<std::string>(child.value);
        // Save the node of this control point
        cp_nodes.push_back(std::make_unique<ASTNode>(child));

        // Create an assignment command 
        auto sem_variable = std::make_unique<Variable>(var);
        auto sem_declaration = std::make_unique<Declaration>(var);
        commands.push_back(std::move(sem_declaration));
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
            auto sem_variable = std::make_unique<Constant>(value);
            auto sem_assignment = std::make_unique<Assignment>(var, std::move(sem_variable));
            commands.push_back(std::move(sem_assignment));
    
        }
        else if(second_child.type == NodeType::VARIABLE){
            std::cout << "\t[Log] Assigning a variable." << std::endl;
            std::string var_name = std::get<std::string>(second_child.value);
            std::cout << "\t[Log] Value: " << var_name << std::endl;

            // Create an assignment command 
            auto sem_variable = std::make_unique<Variable>(var_name);
            auto sem_assignment = std::make_unique<Assignment>(var_name, std::move(sem_variable));
            commands.push_back(std::move(sem_assignment));
        }else if(second_child.type == NodeType::ARITHM_OP){
            std::cout << "\t[Log] Assigning an arithmetic expression." << std::endl;
            std::unique_ptr<BinaryOp> sem_binop = create_binop(second_child);  
            auto sem_assignment = std::make_unique<Assignment>(var, std::move(sem_binop));
            commands.push_back(std::move(sem_assignment));
        }
    }else if(node.type == NodeType::PRE_CON){
        std::cout << "[Log] Pre condition found." << std::endl;
        // Save the node of this control point
        cp_nodes.push_back(std::make_unique<ASTNode>(node));

        // Every Pre condition contains two Logic operation children, referred to 
        // the same variable
        ASTNode left_child = node.children[0];
        ASTNode right_child = node.children[1];
        assert(left_child.type == NodeType::LOGIC_OP);
        assert(right_child.type == NodeType::LOGIC_OP);
        assert(std::get<LogicOp>(left_child.value) == LogicOp::GEQ);
        assert(std::get<LogicOp>(right_child.value) == LogicOp::LEQ);
        ASTNode left_left_child = left_child.children[0];
        ASTNode left_right_child = left_child.children[1];
        assert(left_left_child.type == NodeType::INTEGER);
        assert(left_right_child.type == NodeType::VARIABLE);
        ASTNode right_left_child = right_child.children[0];
        ASTNode right_right_child = right_child.children[1];
        assert(right_left_child.type == NodeType::INTEGER);
        assert(right_right_child.type == NodeType::VARIABLE);
        std::string var_name = std::get<std::string>(left_right_child.value);
        int new_lb = std::get<int>(left_left_child.value);
        int new_ub = std::get<int>(right_left_child.value);
        auto sem_variable = std::make_unique<Variable>(var_name);
        auto sem_assignment = std::make_unique<Assignment>(var_name, std::make_unique<Constant>(new_lb, new_ub));
        commands.push_back(std::move(sem_assignment));
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
        command->execute(stores.back());   
        std::cout << "[Log] Command executed." << std::endl;
        stores.back().print();
        std::cout << std::endl;
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
    int iterations = 0;
    // Solve the equations until the fixed point is reached
    bool fixed_point_reached = false;
    do{
        fixed_point_reached = solve_step();
        iterations++;
    }while(!fixed_point_reached);
    
    std::cout << "[Log] Fixed point reached after " << iterations << " iterations." << std::endl;
}