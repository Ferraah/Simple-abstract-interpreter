#include <fstream>
#include <sstream>

#include "parser.hpp"
#include "ast.hpp"
#include "interval.hpp"
//#include "interval_set.hpp"
#include "disjointed_intervals.hpp"
#include "invariant.hpp"
#include "abstract_interpreter.hpp"


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
    std::cout << std::endl;
    ai.print_invariants(); 
    std::cout << std::endl;
    ai.print_warnings();

    return 0;
}