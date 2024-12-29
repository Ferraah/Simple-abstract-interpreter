#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "invariant.hpp"
#include "interval.hpp"
#include "ast.hpp"
#include <functional>
#include <utility>
#include <algorithm>
#include <vector>
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
                assert(invariant.contains(name));
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

            DisjointedIntervals result;
            switch (op) {
                case BinOp::ADD: result = lval + rval; break;
                case BinOp::SUB: result = lval - rval; break;
                case BinOp::MUL: result = lval * rval; break;
                case BinOp::DIV: {
                    if(rval == DisjointedIntervals(Interval(0,0))){
                    add_warning_to_list("[ERROR] Division by zero detected!");
                    }
                    else if(rval.contains(0))
                    add_warning_to_list("[WARNING] Possible division by zero");
                    result = lval / rval;
                    break;
                }
                default: {
                    throw std::runtime_error("Unknown binary operator");
                }
            }
            return result;
        }
    };

    class ControlPointAction {
        public:
            virtual ~ControlPointAction() = default;
            virtual void execute(std::vector<Invariant> &prev_invariants, std::vector<Invariant> &new_invariants) const = 0; // Execute the action on the current Invariants
    };


    class JoinInvariants : public ControlPointAction {
        std::vector<size_t> control_points;
        size_t target_control_point;

    public:
        explicit JoinInvariants(size_t target_control_point, std::vector<size_t> control_points)
            : control_points(std::move(control_points)), target_control_point(target_control_point) {};

        void execute(std::vector<Invariant> &prev_invariants, std::vector<Invariant> &new_invariants) const { // Execute the action on the current Invariants
            Invariant &target_invariant = new_invariants[target_control_point];

            // Deep copy of first element to join
            target_invariant = Invariant(prev_invariants[control_points[0]]); 
            for(size_t i = 1; i < control_points.size(); i++){
                // Join the others
                target_invariant = target_invariant.join(prev_invariants[control_points[i]]);
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

        // Run a commmand on the invariants at time t to generate the outpoints at time t+1
        void execute(std::vector<Invariant> &prev_invariants, std::vector<Invariant> &next_invariants) const override {
            

            // Modify the new invariant based on the previous ones
            assert(input_cp_id < prev_invariants.size());
            assert(output_cp_id < next_invariants.size());

            // The command takes in input the invariant at location point input_cp_id and saves the result in the invariant at location point output_cp_id
            bool is_zero_invariant = prev_invariants[input_cp_id].get_is_zero_invariant();
            
            if(is_zero_invariant || (prev_invariants[input_cp_id].size() > 0)){
                //std::cout << "Executing command from " << input_cp_id << " to " << output_cp_id << std::endl;
                //std::cout << "Size of the invariant at " << input_cp_id << " is " << prev_invariants[input_cp_id].size() << std::endl;
                execute(prev_invariants[input_cp_id], next_invariants[output_cp_id]);
            }
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

            // Copy the old invariant
            output = input; 

            // Check if the variable has already been declared
            assert(input.contains(variable));

            // Assign a value to a variable already declared in the invariant
            // Evaluate the expression from the variables in the input invariant
            output[variable] = expression->evaluate(input);
        }
    };

    class Declaration : public Command {
        std::string variable;
    public:
        explicit Declaration(const std::string& variable, size_t input_cp_id) : Command(input_cp_id), variable(variable) {}
        explicit Declaration(const std::string& variable, size_t input_cp_id, size_t output_cp_id) : Command(input_cp_id, output_cp_id), variable(variable) {}
        void execute(const Invariant& input, Invariant& output) const override {

            // Copy the old invariant
            output = input;

            // Check if the variable has already been declared
            assert(!input.contains(variable));

            // Update the target value
            output[variable] = DisjointedIntervals(Interval(INT_MIN, INT_MAX));
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

            output = input;
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
            
            output = input;

            DisjointedIntervals left = input[left_variable_name];
            //std::cout << "Left variable " << left_variable_name << " has value " << left << std::endl;
            DisjointedIntervals right = right_expression->evaluate(input);
            // std::cout << "Right expression has value " << right << std::endl;
            // std::cout << "Filtering variable " << left_variable_name << " with operation " << op << " and value " << right << std::endl;
            // std::cout << "Before filtering: " << left << std::endl;

            // Limited to constants for now
            assert(right.ub() == right.lb());

            switch (op){
                case LogicOp::LE:
                    output[left_variable_name] =  output[left_variable_name].meet(Interval(INT_MIN, right.ub()-1));
                    break;
                case LogicOp::LEQ:
                    output[left_variable_name] = output[left_variable_name].meet(Interval(INT_MIN, right.ub()));
                    break;
                case LogicOp::GE:
                    output[left_variable_name] = output[left_variable_name].meet(Interval(right.lb()+1, INT_MAX));
                    break;
                case LogicOp::GEQ:
                    output[left_variable_name] = output[left_variable_name].meet(Interval(right.lb(), INT_MAX));
                    break;
                case LogicOp::EQ:
                    output[left_variable_name].filter_eq(right);
                    break;
                case LogicOp::NEQ:
                    output[left_variable_name].filter_neq(right);
                    break;
            }

        }
    };

        

    
}


#endif