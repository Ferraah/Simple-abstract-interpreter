#ifndef ABSTRACT_INTERPRETER_AST_HPP
#define ABSTRACT_INTERPRETER_AST_HPP

#include <variant>
#include <cmath>
#include <iostream>
#include <atomic>

enum class BinOp {ADD, SUB, MUL, DIV};
inline std::ostream& operator<<(std::ostream& os, BinOp op) {
    switch (op) {
        case BinOp::ADD: os << "+"; break;
        case BinOp::SUB: os << "-"; break;
        case BinOp::MUL: os << "*"; break;
        case BinOp::DIV: os << "/"; break;
    }
    return os;
}

inline std::string to_string(BinOp op) {
    switch (op) {
        case BinOp::ADD: return "+";
        case BinOp::SUB: return "-";
        case BinOp::MUL: return "*";
        case BinOp::DIV: return "/";
        default: return "Unknown";
    }
}

enum class LogicOp {LE, LEQ, GE, GEQ, EQ, NEQ};
inline LogicOp get_opposite(LogicOp lop) {
    switch (lop) {
        case LogicOp::LE: return LogicOp::GEQ;
        case LogicOp::LEQ: return LogicOp::GE;
        case LogicOp::GE: return LogicOp::LEQ;
        case LogicOp::GEQ: return LogicOp::LE;
        case LogicOp::EQ: return LogicOp::NEQ;
        case LogicOp::NEQ: return LogicOp::EQ;
        default: throw std::invalid_argument("Unknown LogicOp");
    }
}
inline std::ostream& operator<<(std::ostream& os, LogicOp lop){
    switch (lop){
        case LogicOp::LE: os << "<"; break;
        case LogicOp::LEQ: os << "<="; break;
        case LogicOp::GE: os << ">"; break;
        case LogicOp::GEQ: os << ">="; break;
        case LogicOp::EQ: os << "=="; break;
        case LogicOp::NEQ: os << "!="; break;
    }
    return os;
}

inline std::string to_string(LogicOp lop) {
    switch (lop) {
        case LogicOp::LE: return "<";
        case LogicOp::LEQ: return "<=";
        case LogicOp::GE: return ">";
        case LogicOp::GEQ: return ">=";
        case LogicOp::EQ: return "==";
        case LogicOp::NEQ: return "!=";
        default: return "Unknown";
    }
}

enum class NodeType {VARIABLE, INTEGER, PRE_CON, POST_CON, ARITHM_OP, LOGIC_OP, DECLARATION, ASSIGNMENT, IFELSE, WHILELOOP, SEQUENCE};
inline std::ostream& operator<<(std::ostream& os, NodeType type) {
    switch (type) {
        case NodeType::VARIABLE: os << "Variable"; break;
        case NodeType::INTEGER: os << "Integer"; break;
        case NodeType::PRE_CON: os << "Pre conditions"; break;
        case NodeType::POST_CON: os << "Post conditions"; break;
        case NodeType::ARITHM_OP: os << "Arithmetic Operation"; break;
        case NodeType::LOGIC_OP: os << "Logic Operation"; break;
        case NodeType::DECLARATION: os << "Declaration"; break;
        case NodeType::ASSIGNMENT: os << "Assignment"; break;
        case NodeType::IFELSE: os << "If-Else"; break;
        case NodeType::WHILELOOP: os << "While-Loop"; break;
        case NodeType::SEQUENCE: os << "Sequence"; break;
    }
    return os;
}
inline std::string to_string(NodeType type) {
    switch (type) {
        case NodeType::VARIABLE: return "Variable";
        case NodeType::INTEGER: return "Integer";
        case NodeType::PRE_CON: return "Pre conditions";
        case NodeType::POST_CON: return "Post conditions";
        case NodeType::ARITHM_OP: return "Arithmetic Operation";
        case NodeType::LOGIC_OP: return "Logic Operation";
        case NodeType::DECLARATION: return "Declaration";
        case NodeType::ASSIGNMENT: return "Assignment";
        case NodeType::IFELSE: return "If-Else";
        case NodeType::WHILELOOP: return "While-Loop";
        case NodeType::SEQUENCE: return "Sequence";
        default: return "Unknown";
    }
}

struct ASTNode {
    using VType = std::variant<std::string, int, BinOp, LogicOp>;
    using ASTNodes = std::vector<ASTNode>;

    static std::atomic<size_t> id_counter;
    size_t id;
    NodeType type;
    VType value;
    ASTNodes children;

    ASTNode(): id(id_counter++), type(NodeType::INTEGER), value(0) {}
    ASTNode(const std::string& name): id(id_counter++), type(NodeType::VARIABLE), value(name){}
    ASTNode(const int num): id(id_counter++), type(NodeType::INTEGER), value(num) {}
    ASTNode(BinOp bop, ASTNode left, ASTNode right)
        : id(id_counter++), type(NodeType::ARITHM_OP), value(bop){
            children.push_back(left);
            children.push_back(right);
        }
    ASTNode(LogicOp lop, ASTNode left, ASTNode right)
        : id(id_counter++), type(NodeType::LOGIC_OP), value(lop){
            children.push_back(left);
            children.push_back(right);
        }
    ASTNode(NodeType t): id(id_counter++), type(t){}
    ASTNode(NodeType t, const std::string& name): id(id_counter++), type(t), value(name){}
    ASTNode(NodeType t, const VType& value): id(id_counter++), type(t), value(value) {}

    static void printVariant(const std::variant<std::string, int, BinOp, LogicOp>& value) {
        std::visit([](const auto& v) {
            std::cout << v << std::endl;
        }, value);
    }

    void print(int depth = 0) const {
        std::string indent(depth * 2, ' ');
        std::cout << indent << "ID: " << id << ", NodeType: " << type << ", Value: ";
        printVariant(value);
        for (const auto& child : children) {
            child.print(depth + 1);
        }
    }
};

inline std::atomic<size_t> ASTNode::id_counter{0};

#endif
