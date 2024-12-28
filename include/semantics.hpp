#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "invariant.hpp"
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
            virtual DisjointedIntervals evaluate(const Invariant& invariant) const = 0; // Evaluate the expression
    };

    class BoolExpr {
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
        LogicOp op;
        public:
        explicit BoolExpr(LogicOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) : left(std::move(left)), right(std::move(right)), op(op) {}
            ~BoolExpr() = default;
            bool evaluate(const Invariant& invariant){
                DisjointedIntervals lval = left->evaluate(invariant);
                DisjointedIntervals rval = right->evaluate(invariant);
                switch (op) {
                    case LogicOp::LE: return lval <= rval;
                    case LogicOp::LEQ: return lval <= rval;
                    case LogicOp::GE: return lval >= rval;
                    case LogicOp::GEQ: return lval >= rval;
                    case LogicOp::EQ: return lval == rval;
                    case LogicOp::NEQ: return lval != rval;
                    default: {
                        throw std::runtime_error("Unknown binary operator");
                    }
                }
            }; // Evaluate the expression
    };

    class Constant : public Expr {

        int left_value;
        int right_value;

        public:
            explicit Constant(int left_value, int right_value) : left_value(left_value), right_value(right_value) {}
            explicit Constant(int value) : left_value(value), right_value(value) {}
            DisjointedIntervals evaluate(const Invariant& invariant) const override {
                return DisjointedIntervals(left_value, right_value);
            }
    };

    class Variable : public Expr {

        std::string name;

        public:
            explicit Variable(const std::string& name) : name(name) {}
            DisjointedIntervals evaluate(const Invariant& invariant) const override {
                return invariant[name];
            }
    };


    class BinaryOp : public Expr {

        BinOp op; // e.g., '+', '-', '*', '/'
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
        std::function<void(std::string)> add_warning_to_list;

    public:
        BinaryOp(BinOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, std::function<void(std::string)> add_warning_to_list)
            : op(op), left(std::move(left)), right(std::move(right)), add_warning_to_list(add_warning_to_list) {}

        DisjointedIntervals evaluate(const Invariant& invariant) const override {
            DisjointedIntervals lval = left->evaluate(invariant);
            DisjointedIntervals rval = right->evaluate(invariant);
            switch (op) {
                case BinOp::ADD: return lval + rval;
                case BinOp::SUB: return lval - rval;
                case BinOp::MUL: return lval * rval;
                case BinOp::DIV: {

                    if(rval == DisjointedIntervals(Interval(0,0))){
                        add_warning_to_list("[ERROR] Division by zero detected!");
                    }
                    else if(rval.contains(0))
                        add_warning_to_list("[WARNING] Possible division by zero");
                    return lval / rval;
                }
                default: {
                    throw std::runtime_error("Unknown binary operator");
                }
            }
        }
    };

    class ControlPointAction {
        public:
            virtual ~ControlPointAction() = default;
            virtual void execute(std::vector<Invariant> &invariants) const = 0; // Execute the action on the current Invariants
    };


    class JoinInvariants : public ControlPointAction {
        std::vector<size_t> control_points;
        size_t target_control_point;

    public:
        explicit JoinInvariants(size_t target_control_point, std::vector<size_t> control_points)
            : control_points(std::move(control_points)), target_control_point(target_control_point) {};

        void execute(std::vector<Invariant> &invariants) const override {
            Invariant &target_invariant = invariants[target_control_point];

            // Deep copy of first element to join
            target_invariant = Invariant(invariants[control_points[0]]); 
            for(size_t i = 1; i < control_points.size(); i++){
                // Join the others
                target_invariant = target_invariant.join(invariants[control_points[i]]);
            }
        }
    };

    class Command : public ControlPointAction{
       protected:
           size_t input_cp_id; 
           size_t output_cp_id;
    public:
        Command(size_t input_cp_id, size_t output_cp_id) : input_cp_id(input_cp_id), output_cp_id(output_cp_id){}
        Command(size_t input_cp_id) : input_cp_id(input_cp_id), output_cp_id(input_cp_id < 0 ? 0 : input_cp_id+1){}

        virtual ~Command() = default;
        virtual void execute(const Invariant& input, Invariant& output) const = 0; // Execute the command

        void execute(std::vector<Invariant> &invariants) const override {
            //std::cout << "Executing command from " << input_cp_id << " to " << output_cp_id << std::endl;
            assert(invariants.size() > 0);
            assert(input_cp_id < invariants.size());
            assert(output_cp_id < invariants.size());
            invariants[output_cp_id] = Invariant(invariants[input_cp_id]);
            execute(invariants[input_cp_id], invariants[output_cp_id]);
        }
    };

    class Assignment : public Command {
        std::string variable;
        std::unique_ptr<Expr> expression;
    public:
        Assignment(const std::string& variable, std::unique_ptr<Expr> expression, size_t input_cp_id)
            : Command(input_cp_id), variable(variable), expression(std::move(expression)) {}
        Assignment(const std::string& variable, std::unique_ptr<Expr> expression, size_t input_cp_id, size_t output_cp_id)
            : Command(input_cp_id, output_cp_id), variable(variable), expression(std::move(expression)) {}
        void execute(const Invariant& input, Invariant& output) const override {
            output[variable] = expression->evaluate(input);
        }
    };

    class Declaration : public Command {
        std::string variable;
    public:
        explicit Declaration(const std::string& variable, size_t input_cp_id) : Command(input_cp_id), variable(variable) {}
        explicit Declaration(const std::string& variable, size_t input_cp_id, size_t output_cp_id) : Command(input_cp_id, output_cp_id), variable(variable) {}
        void execute(const Invariant& input, Invariant& output) const override {
            output[variable] = DisjointedIntervals(Interval());
        }
    };


    class Assert : public Command {
        std::unique_ptr<BoolExpr> expression;
        std::function<void(std::string)> add_warning_to_list;
    public:
        explicit Assert(std::unique_ptr<BoolExpr> expression, std::function<void(std::string)> add_warning_to_list, size_t input_cp_id)
            : Command(input_cp_id), expression(std::move(expression)), add_warning_to_list(add_warning_to_list) {}
        explicit Assert(std::unique_ptr<BoolExpr> expression, std::function<void(std::string)> add_warning_to_list, size_t input_cp_id, size_t output_cp_id)
            : Command(input_cp_id, output_cp_id), expression(std::move(expression)), add_warning_to_list(add_warning_to_list) {}
        void execute(const Invariant& input, Invariant& output) const override {
            bool result = expression->evaluate(input);
            if(!result){
                add_warning_to_list("[ERROR] Assertion failed!");
            }
        }
    };


    class Filter : public Command {
        std::string left_variable_name;
        std::unique_ptr<Expr> right_expression;
        LogicOp op;
    public:
        explicit Filter(LogicOp op, const std::string& left_variable_name, std::unique_ptr<Expr> right_expression, size_t input_cp_id) : left_variable_name(left_variable_name), right_expression(std::move(right_expression)), op(op), Command(input_cp_id) {}
        explicit Filter(LogicOp op, const std::string& left_variable_name, std::unique_ptr<Expr> right_expression, size_t input_cp_id, size_t output_cp_id) : left_variable_name(left_variable_name), right_expression(std::move(right_expression)), op(op), Command(input_cp_id, output_cp_id) {}

        void execute(const Invariant& input, Invariant& output) const override{
            DisjointedIntervals left = input[left_variable_name];
            std::cout << "Left variable " << left_variable_name << " has value " << left << std::endl;
            DisjointedIntervals right = right_expression->evaluate(input);
            std::cout << "Right expression has value " << right << std::endl;
            std::cout << "Filtering variable " << left_variable_name << " with operation " << op << " and value " << right << std::endl;
            std::cout << "Before filtering: " << left << std::endl;
            switch (op){
                case LogicOp::LE:
                    output[left_variable_name].filter_l(right);
                    break;
                case LogicOp::LEQ:
                    output[left_variable_name].filter_leq(right);
                    break;
                case LogicOp::GE:
                    output[left_variable_name].filter_g(right);
                    break;
                case LogicOp::GEQ:
                    output[left_variable_name].filter_geq(right);
                    break;
                case LogicOp::EQ:
                    output[left_variable_name].filter_eq(right);
                    break;
                case LogicOp::NEQ:
                    output[left_variable_name].filter_neq(right);
                    break;
            }

            std::cout << "After filtering: " << output[left_variable_name] << std::endl;
        }
    };

        
/*
    class Filter : public Command {
        std::string variable;
        DisjointedIntervals DisjointedIntervals_filter;
        LogicOp op;
    public:
        explicit Filter(LogicOp op, const std::string& variable, DisjointedIntervals DisjointedIntervals_filter) : variable(variable), DisjointedIntervals_filter(DisjointedIntervals_filter), op(op) {}

        void execute(Invariant& invariant) const override{

            switch (op){
                case LogicOp::LE:
                    invariant[variable].
                    break;
                case LogicOp::LEQ:
                    invariant[variable] = 
                    break;
                case LogicOp::GE:
                    invariant[variable] = invariant[variable].filter_ge(val);
                    break;
                case LogicOp::GEQ:
                    invariant[variable] = invariant[variable].filter_geq(val);
                    break;
                case LogicOp::EQ:
                    invariant[variable] = invariant[variable].filter_eq(val);
                    break;
                case LogicOp::NEQ:
                    invariant[variable] = invariant[variable].filter_neq(val);
                    break;
            }
        }
    }
*/
    
}


#endif