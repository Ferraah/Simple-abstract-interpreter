#include <fstream>
#include <sstream>

#include "parser.hpp"
#include "ast.hpp"
#include "interval.hpp"
#include "store.hpp"
#include "abstract_interpreter.hpp"

bool check_valid_divisions(const ASTNode& node) {

    std::cout << "Checking node." << std::endl;

    // If the node is a division, we need to check if the right child is zero
    if(node.type == NodeType::ARITHM_OP && std::get<BinOp>(node.value) == BinOp::DIV){
        std::cout << "Found division." << std::endl;
        NodeType right_type = node.children[1].type;
        std::cout << "Right child type: " << right_type << std::endl;
        if((right_type == NodeType::VARIABLE || right_type == NodeType::INTEGER)){
            if(std::get<int>(node.children[1].value) == 0){
                std::cout << "Division by zero detected!" << std::endl;
                return false;
            }
        }
    }

    // On leaf nodes, we don't need to check anything 
    if(node.children.empty())
        return true;

    // Check all children 
    bool valid;

    for(const auto& child : node.children) {
        valid = check_valid_divisions(child);
        // If one of children is invalid, we can return false
        if(!valid)
            return false;
    }

    return true;


}



int main(int argc, char** argv) {
    if(argc != 2) {
        std::cout << "usage: " << argv[0] << " tests/00.c" << std::endl;
    }
    std::ifstream f(argv[1]);
    if (!f.is_open()){
        std::cerr << "[ERROR] cannot open the test file `" << argv[1] << "`." << std::endl;
        return 1;
    }
    std::ostringstream buffer;
    buffer << f.rdbuf();
    std::string input = buffer.str();
    f.close();

    std::cout << "Parsing program `" << argv[1] << "`..." << std::endl;
    AbstractInterpreterParser AIParser;
    ASTNode ast = AIParser.parse(input);
    ast.print();

    AbstractInterpreter ai;
    ai.init_equations(ast);     

    ai.solve_equations();
    ai.print_store();

    return 0;
}