#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "store.hpp"
#include "interval.hpp"
#include <functional>
#include <utility>
#include <algorithm>
#include <memory>

namespace semantics {


    class Expr {
        public:
            virtual ~Expr() = default;
            virtual Interval evaluate(const Store& store) const = 0; // Evaluate the expression
    };

    class Constant : public Expr {

        int value;

        public:
            explicit Constant(int value) : value(value) {}
            Interval evaluate(const Store& store) const override {
                return Interval(value, value);
            }
    };

    class Variable : public Expr {

        std::string name;

        public:
            explicit Variable(const std::string& name) : name(name) {}
            Interval evaluate(const Store& store) const override {
                return store[name];
            }
    };


    class BinaryOp : public Expr {
        char op; // e.g., '+', '-', '*', '/'
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

    public:
        BinaryOp(char op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
            : op(op), left(std::move(left)), right(std::move(right)) {}

        Interval evaluate(const Store& store) const override {
            Interval lval = left->evaluate(store);
            Interval rval = right->evaluate(store);
            switch (op) {
                case '+': return lval + rval;
                case '-': return lval - rval;
                case '*': return lval * rval;
                case '/': return lval / rval;
                default: throw std::runtime_error("Unsupported operator");
            }
        }
    };


    class Command {
    public:
        virtual ~Command() = default;
        virtual void execute(Store& store) const = 0; // Execute the command
    };

    class Assignment : public Command {
        std::string variable;
        std::unique_ptr<Expr> expression;

    public:
        Assignment(const std::string& variable, std::unique_ptr<Expr> expression)
            : variable(variable), expression(std::move(expression)) {}

        void execute(Store& store) const override {
            store[variable] = expression->evaluate(store);
        }
    };

    class Declaration : public Command {
        std::string variable;
    public:
        explicit Declaration(const std::string& variable) : variable(variable) {}

        void execute(Store& store) const override {
            store[variable] = Interval();
        }
    };


    
}


#endif