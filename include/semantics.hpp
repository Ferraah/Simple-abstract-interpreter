#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "store.hpp"
#include "interval.hpp"
#include "ast.hpp"
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

        int left_value;
        int right_value;

        public:
            explicit Constant(int left_value, int right_value) : left_value(left_value), right_value(right_value) {}
            explicit Constant(int value) : left_value(value), right_value(value) {}
            Interval evaluate(const Store& store) const override {
                return Interval(left_value, right_value);
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
        BinOp op; // e.g., '+', '-', '*', '/'
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

    public:
        BinaryOp(BinOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
            : op(op), left(std::move(left)), right(std::move(right)) {}

        Interval evaluate(const Store& store) const override {
            Interval lval = left->evaluate(store);
            Interval rval = right->evaluate(store);
            switch (op) {
                case BinOp::ADD: return lval + rval;
                case BinOp::SUB: return lval - rval;
                case BinOp::MUL: return lval * rval;
                case BinOp::DIV: return lval / rval;
                default: {
                    throw std::runtime_error("Unknown binary operator");
                }
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
/*
    class Filter : public Command {
        std::string variable;
        Interval Interval_filter;
        LogicOp op;
    public:
        explicit Filter(LogicOp op, const std::string& variable, Interval Interval_filter) : variable(variable), Interval_filter(Interval_filter), op(op) {}

        void execute(Store& store) const override{

            switch (op){
                case LogicOp::LE:
                    store[variable].
                    break;
                case LogicOp::LEQ:
                    store[variable] = 
                    break;
                case LogicOp::GE:
                    store[variable] = store[variable].filter_ge(val);
                    break;
                case LogicOp::GEQ:
                    store[variable] = store[variable].filter_geq(val);
                    break;
                case LogicOp::EQ:
                    store[variable] = store[variable].filter_eq(val);
                    break;
                case LogicOp::NEQ:
                    store[variable] = store[variable].filter_neq(val);
                    break;
            }
        }
    }
*/
    
}


#endif