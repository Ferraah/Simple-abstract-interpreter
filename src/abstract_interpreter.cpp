#include "abstract_interpreter.hpp"


std::shared_ptr<semantics::BinaryOp> AbstractInterpreter::create_binop(const ASTNode& node, std::function<void(std::string)> add_warning_to_list) {
    using namespace semantics;

    // Retrieve the operation from the AST
    BinOp op = std::get<BinOp>(node.value);
    
    // Node children, aka the operands
    ASTNode left_child = node.children[0];
    ASTNode right_child = node.children[1];

    // Create the expressions for the operands

    std::shared_ptr<Expr> left_expr = nullptr;
    if (left_child.type == NodeType::INTEGER){
        left_expr = std::make_shared<Constant>(std::get<int>(left_child.value));
    }else if(left_child.type == NodeType::VARIABLE){
        left_expr = std::make_shared<Variable>(std::get<std::string>(left_child.value));
    }else if(left_child.type == NodeType::ARITHM_OP){
        // Recursive call to create the expression
        left_expr = create_binop(left_child, add_warning_to_list);
    }

    std::shared_ptr<Expr> right_expr = nullptr;
    if (right_child.type == NodeType::INTEGER){
        right_expr = std::make_shared<Constant>(std::get<int>(right_child.value));
    }else if(right_child.type == NodeType::VARIABLE){
        right_expr = std::make_shared<Variable>(std::get<std::string>(right_child.value));
    }else if(right_child.type == NodeType::ARITHM_OP){
        // Recursive call to create the expression
        right_expr = create_binop(right_child, add_warning_to_list);
    }

    assert(left_expr != nullptr);
    assert(right_expr != nullptr);
    
    std::cout << "\t\t[Log] Creating binary operation with op: "<< op << std::endl;



    return std::make_shared<BinaryOp>(op, left_expr, right_expr, add_warning_to_list);
}

void AbstractInterpreter::init_equations(const ASTNode& node) {
    using namespace semantics;

    bool continue_recursion = true;

    if(node.type == NodeType::DECLARATION){
        std::cout << "[Log] Declaration found." << std::endl;
        ASTNode child = node.children[0];
        assert(child.type == NodeType::VARIABLE);
        std::cout << "\t[Log] Variable found." << std::endl;
        std::string var = std::get<std::string>(child.value);

        // Create an assignment command 
        auto sem_variable = std::make_shared<Variable>(var);
        auto sem_declaration = std::make_shared<Declaration>(var, solver_components.size());
        solver_components.push_back(std::move(sem_declaration));

    }
    
    if(node.type == NodeType::ASSIGNMENT){
        std::cout << "[Log] Assignment found." << std::endl;

        // Name of the variable we are assigning to the value 
        std::string var = std::get<std::string>(node.children[0].value);

        ASTNode second_child = node.children[1];
        if(second_child.type == NodeType::INTEGER){
            std::cout << "\t[Log] Assigning an integer." << std::endl;
            int value = std::get<int>(second_child.value);
            std::cout << "\t[Log] Value: " << value << std::endl;

            // Create an assignment command 
            auto sem_constant = std::make_shared<Constant>(value);
            auto sem_assignment = std::make_shared<Assignment>(var, sem_constant, solver_components.size());
            solver_components.push_back(std::move(sem_assignment));
    
        }
        else if(second_child.type == NodeType::VARIABLE){
            std::cout << "\t[Log] Assigning a variable." << std::endl;
            std::string var_name = std::get<std::string>(second_child.value);
            std::cout << "\t[Log] Value: " << var_name << std::endl;

            // Create an assignment command 
            auto sem_variable = std::make_shared<Variable>(var_name);
            auto sem_assignment = std::make_shared<Assignment>(var_name, sem_variable, solver_components.size());
            solver_components.push_back(std::move(sem_assignment));


        }else if(second_child.type == NodeType::ARITHM_OP){
            std::cout << "\t[Log] Assigning an arithmetic expression." << std::endl;

            auto add_warning_to_list = [this, second_child](std::string warning) {
                // Node description
                std::cout << "[Log] Warning added to the list." << std::endl;
                //std::string node_description = "NodeType: Arithmetic Operation, Value: "+ to_string(op);
                this->warnings_list[second_child.id] = warning;
                
            };

            std::shared_ptr<BinaryOp> sem_binop = create_binop(second_child, add_warning_to_list);  
            auto sem_assignment = std::make_shared<Assignment>(var, sem_binop, solver_components.size());
            solver_components.push_back(std::move(sem_assignment));
        }


    }
    else if(node.type == NodeType::PRE_CON){

        std::cout << "[Log] Pre condition found." << std::endl;
        // Save the node of this control point

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
        auto sem_variable = std::make_shared<Variable>(var_name);
        auto sem_constant = std::make_shared<Constant>(new_lb, new_ub);
        auto sem_assignment = std::make_shared<Assignment>(var_name, sem_constant, solver_components.size());
        solver_components.push_back(std::move(sem_assignment));

    }else if(node.type == NodeType::POST_CON){
        std::cout << "[Log] Post condition found." << std::endl;
        // Save the node of this control point
        ASTNode child = node.children[0];
        assert(child.type == NodeType::LOGIC_OP);
        LogicOp op = std::get<LogicOp>(child.value);
        ASTNode left_child = child.children[0];
        ASTNode right_child = child.children[1];


        auto add_warning_to_list = [this, node](std::string warning) {
            // Node description
            std::cout << "[Log] Warning added to the list." << std::endl;
            //std::string node_description = "NodeType: Arithmetic Operation, Value: "+ to_string(op);
            this->warnings_list[node.id] = warning;
        };


        std::vector<std::shared_ptr<Expr>> exprs(2);
        if(left_child.type == NodeType::VARIABLE){
            std::string var_name = std::get<std::string>(left_child.value);
            exprs[0] = std::make_shared<Variable>(var_name);
        }else if(left_child.type == NodeType::INTEGER){
            int value = std::get<int>(left_child.value);
            exprs[0] = std::make_shared<Constant>(value);
        }else if(left_child.type == NodeType::ARITHM_OP){  
            auto sem_binop = create_binop(left_child, add_warning_to_list);
            exprs[0] = std::move(sem_binop);
        }

        if(right_child.type == NodeType::VARIABLE){
            std::string var_name = std::get<std::string>(right_child.value);
            exprs[1] = std::make_shared<Variable>(var_name);
        }else if(right_child.type == NodeType::INTEGER){
            int value = std::get<int>(right_child.value);
            exprs[1] = std::make_shared<Constant>(value);
        }else if(right_child.type == NodeType::ARITHM_OP){  
            auto sem_binop = create_binop(right_child, add_warning_to_list);
            exprs[1] = std::move(sem_binop);
        }

        auto sem_bool_expr = std::make_shared<BoolExpr>(op, 
            exprs[0], 
            exprs[1]);

        auto sem_assert = std::make_shared<Assert>(sem_bool_expr, add_warning_to_list, solver_components.size());
        solver_components.push_back(std::move(sem_assert));
    }
    else if(node.type == NodeType::IFELSE){
        if(std::get<std::string>(node.value) == "IfElse"){
            std::cout << "[Log] If-Else found." << std::endl;

            // ROOT of IFELSE
            ASTNode condition = node.children[0];
            ASTNode if_body = node.children[1];

            // Retrieve the logic operation of the IF condition
            ASTNode logic_op_node = condition.children[0];
            assert(logic_op_node.type == NodeType::LOGIC_OP);
            assert(logic_op_node.children[0].type == NodeType::VARIABLE);
            LogicOp logic_op = std::get<LogicOp>(logic_op_node.value);

            // Retrieve the left variable name
            std::string left_var_name = std::get<std::string>(logic_op_node.children[0].value);

            // Retrieve the right expression
            // FOR NOW, WE ONLY CONSIDER THE CASE WHERE THE RIGHT EXPRESSION IS AN INTEGER
            assert(logic_op_node.children[1].type == NodeType::INTEGER);
            auto right_expr = std::make_shared<Constant>(std::get<int>(logic_op_node.children[1].value));
            auto right_expr_else = std::make_shared<Constant>(std::get<int>(logic_op_node.children[1].value));

            // Get the last location point, before running the branches 
            size_t before_branch_id = solver_components.size();

            // Create the filter command for the IF condition
            auto sem_filter_if = std::make_shared<Filter>(logic_op, left_var_name, right_expr, before_branch_id);
            solver_components.push_back(sem_filter_if);

            // Evaluate the if body
            for(const auto& child : if_body.children){
                init_equations(child);
            }

            // Save the last control point id of the if body to later join it with the else body
            size_t if_last_cp_id = solver_components.size();
            std::cout << "[Log] If body last control point id: " << if_last_cp_id << std::endl;
            
            // Check if there is an else branch
            if(node.children.size() == 3){
                ASTNode else_body = node.children[2];

                // Create the filter command for the ELSE condition
                auto sem_filter_else = std::make_shared<Filter>(get_opposite(logic_op), left_var_name, right_expr_else, before_branch_id, if_last_cp_id+1);
                solver_components.push_back(sem_filter_else);

                // Evaluate the else body
                for(const auto& child : else_body.children){
                    init_equations(child);
                }

                size_t else_last_cp_id = solver_components.size();
                std::cout << "[Log] Else body last control point id: " << else_last_cp_id << std::endl;

                // Now we merge the two branches, creating a new invariant that is the join of the last invariants for each branch
                auto sem_unify = std::make_shared<JoinInvariants>(else_last_cp_id+1, std::vector<size_t>{if_last_cp_id, else_last_cp_id});
                solver_components.push_back(sem_unify);
            }
            else{
                // If there is no else branch, we just need to unify the last control point of the if branch
                auto sem_unify = std::make_shared<JoinInvariants>(if_last_cp_id+1, std::vector<size_t>{before_branch_id, if_last_cp_id});
                solver_components.push_back(sem_unify);
            }
            return;
        }
    }
    else if(node.type == NodeType::WHILELOOP){
        if(std::get<std::string>(node.value) == "WhileLoop"){
            std::cout << "[Log] While loop found." << std::endl;

            // ROOT of WHILELOOP
            ASTNode condition = node.children[0];
            ASTNode body = node.children[1];

            // Retrieve the logic operation of the WHILE condition
            ASTNode logic_op_node = condition.children[0];
            assert(logic_op_node.type == NodeType::LOGIC_OP);
            assert(logic_op_node.children[0].type == NodeType::VARIABLE);
            LogicOp logic_op = std::get<LogicOp>(logic_op_node.value);

            // Retrieve the left variable name
            std::string left_var_name = std::get<std::string>(logic_op_node.children[0].value);

            // Retrieve the right expression
            // FOR NOW, WE ONLY CONSIDER THE CASE WHERE THE RIGHT EXPRESSION IS AN INTEGER
            assert(logic_op_node.children[1].type == NodeType::INTEGER);
            auto right_expr = std::make_shared<Constant>(std::get<int>(logic_op_node.children[1].value));

            
            // Get the last location point, before running the branches 
            size_t before_branch_id = solver_components.size(); // L0

            // Next invariant will be a join of LO and the last one in the body. We will substitute this command later, this is just a placeholder 
            // to keep track of the control point id
            solver_components.push_back(nullptr);
            size_t join_cp_id = solver_components.size(); // L1

            // Create the filter command for the WHILE condition
            // Branch on the variable condition, for the while body
            solver_components.push_back(std::make_shared<Filter>(logic_op, left_var_name, right_expr, join_cp_id)); // L2

            // Evaluate the body, adding all the block solver_components to the list
            for(const auto& child : body.children){
                init_equations(child);
            }
            // Save the last control point id of the body to later join it with the condition
            size_t body_last_cp_id = solver_components.size(); // LK (last control point of the body)
            std::cout << "[Log] Body last control point id: " << body_last_cp_id << std::endl;

            // Substitutes the incomplete L1 command with the complete one
            // -1 because the list of solver_components starts from the 1 control point and not 0
            solver_components[join_cp_id-1] = std::make_shared<JoinInvariants>(join_cp_id, std::vector<size_t>{before_branch_id, body_last_cp_id}); // L1 = L0 U LK

            // Create the inverse filter command for the exit of while condition
            solver_components.push_back(std::make_shared<Filter>(get_opposite(logic_op), left_var_name, right_expr, join_cp_id, solver_components.size()+1)); // LK+1
            return;
        }
    }
    

    for(const auto& child : node.children){
        init_equations(child);
    }
    
}


/**
 * @return True if the fixed point is reached, false otherwise
 */
bool AbstractInterpreter::solve_step() {


    InvariantsSystem new_invariants(solver_components.size()+1, Invariant());
    new_invariants[0].set_zero_invariant(true);

    // Executing F, i.e running each of its component
    for(auto& command : solver_components){
        command->execute(invariants, new_invariants);   
    }

    // Check fixed point reached
    bool result = invariants == new_invariants;

    invariants = new_invariants;
    return result; 
}

void AbstractInterpreter::solve_equations() {

    std::cout << "[Log] Number of solver_components: " << solver_components.size() << std::endl;

    // For each control point, we have created an action that will be executed and will modify that control point invariant.
    // We start from the first control point, which is the initial state of the program and contains an invariant marked as 
    // contaning all possible assignments
    invariants = InvariantsSystem(solver_components.size()+1, Invariant());
    invariants[0].set_zero_invariant(true);

    int iterations = 0;
    // Solve the equations until the fixed point is reached
    bool fixed_point_reached = false;
    do{
        std::cout << "[Log] Solving step: " << iterations << std::endl;
        fixed_point_reached = solve_step();
        iterations++;
    }while(!fixed_point_reached);
    
    std::cout << "[Log] Fixed point reached after " << iterations << " iterations." << std::endl;
}