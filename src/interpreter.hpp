#pragma once
#include<string>
#include<memory>
#include<vector>
#include<sstream>
#include "Expr.hpp"
#include "token.hpp"
#include "Environment.hpp"
#include "RuntimeError.hpp"
#include "LoxCallable.hpp"
#include "LoxClock.hpp"
#include "LoxFunction.hpp"
#include "ReturnException.hpp"

class Interpreter : public ExprVisitorEval, public StmtVisitorPrint {
public:
    Interpreter() {
        globals->define("clock", std::make_shared<LoxClock>());
        environment = globals;
    }

    void visit(const Expression& stmt) const override {
        evaluate(*stmt.expression);
    }

    void visit(const Print& stmt) const override {
        literal value = evaluate(*stmt.expression);
        std::cout<< literal_to_string(value) << std::endl;
    }

    void execute(Stmt& stmt) const {
        stmt.accept(*this);
    }

    literal evaluate(const Expr& expr) const {
        return expr.accept(*this);
    }

    literal visit(const Literal& expr) const override{
        // Convert expr.value (variant without callable) to literal (variant with callable)
        // by visiting and reconstructing the correct type
        return std::visit([](auto&& arg) -> literal {
            using T = std::decay_t<decltype(arg)>;
            return literal(arg);
        }, expr.value);
    }

    literal visit(const Grouping& expr) const override {
        return evaluate(*expr.expression);
    }

    literal visit(const Variable& expr) const override {
        return environment->getValue(expr.name);
    }

    literal visit(const Logical& expr) const override {
        literal left = evaluate(*expr.left);
        if(expr.op.getType() == TokenType::OR){
            if(isTruthy(left)) return left;
        } else if(expr.op.getType() == TokenType::AND){
            if(!isTruthy(left)) return left;
        }
        // Only evaluate right if needed (short-circuit)
        return evaluate(*expr.right);
    }

    void visit(const Var& stmt) const override {
        literal value;
        if(stmt.initializer != nullptr){
            value = evaluate(*stmt.initializer);
        } else {
            value = std::monostate{};
        }
        environment->define(stmt.name.getLexeme(), value);
    }

    literal visit(const Binary& expr) const override {
        literal left = evaluate(*expr.left);
        literal right = evaluate(*expr.right);
        switch(expr.op.getType()){
            case TokenType::PLUS:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                if(std::holds_alternative<double>(left) && std::holds_alternative<double>(right)){
                    return std::get<double>(left) + std::get<double>(right);
                }
                if(std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)){
                    return std::get<std::string>(left) + std::get<std::string>(right);
                }
                throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
            case TokenType::MINUS:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) - std::get<double>(right);
            case TokenType::STAR:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) * std::get<double>(right);
            case TokenType::SLASH:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) / std::get<double>(right);
            case TokenType::EQUAL_EQUAL:
                return left == right;
            case TokenType::BANG_EQUAL:
                return left != right;
            case TokenType::GREATER:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) > std::get<double>(right);
            case TokenType::GREATER_EQUAL:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) >= std::get<double>(right);
            case TokenType::LESS:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) < std::get<double>(right);
            case TokenType::LESS_EQUAL:
                if(std::holds_alternative<std::shared_ptr<LoxCallable>>(left) || std::holds_alternative<std::shared_ptr<LoxCallable>>(right))
                    throw RuntimeError(expr.op, "Operands must not be functions.");
                checkNumberOperand(expr.op, {left, right});
                return std::get<double>(left) <= std::get<double>(right);
        }
        return std::monostate{};
    }

    literal visit(const Unary& expr) const override {
        literal right = evaluate(*expr.right);
        switch(expr.op.getType()){
            case TokenType::MINUS:
                checkNumberOperand(expr.op, {right});
                return -(std::get<double>(right));
            case TokenType::BANG:
                return !isTruthy(right);
        }  
        return std::monostate{};
    }

    literal visit(const Assign& expr) const override {
        literal value = evaluate(*expr.value);
        environment->assign(expr.name, value);
        return value;
    }

    literal visit(const Call& expr) const override {
        literal callee = evaluate(*expr.callee);
        std::vector<literal> arguments;

        for(const auto& arg : expr.arguments){
            arguments.push_back(evaluate(arg));
        }
        if(!std::holds_alternative<std::shared_ptr<LoxCallable>>(callee)){
            throw RuntimeError(expr.paren, "Can only call functions and classes.");
        }
        auto functionPtr = std::get_if<std::shared_ptr<LoxCallable>>(&callee);
        if(functionPtr == nullptr || !(*functionPtr)){
            throw RuntimeError(expr.paren, "Undefined function.");
        }
        if(arguments.size() != (*functionPtr)->arity()){
            throw RuntimeError(expr.paren, "Expected " + std::to_string((*functionPtr)->arity()) + " arguments but got " + std::to_string(arguments.size()) + ".");
        }

        return (*functionPtr)->call(*this, arguments);
    }

    void visit(const Function& stmt) const override {
        auto function = std::make_shared<LoxFunction>(stmt, std::make_shared<Environment>(environment));
        environment->define(stmt.name.getLexeme(), function);
        return;
    }

    void visit(const Return& stmt) const override {
        literal value;
        if(stmt.value != nullptr){
            value = evaluate(*stmt.value);
        } else {
            value = std::monostate{};
        }
        throw ReturnException(value);
    }

    void visit(const Block& stmt) const override {
        executeBlock(stmt.statements, std::make_shared<Environment>(environment));
        return;
    }

    void visit(const If& stmt) const override {
        literal condition = evaluate(*stmt.condition);
        if(isTruthy(condition)){
            if (stmt.thenBranch) execute(*stmt.thenBranch);
        }else if(stmt.elseBranch != nullptr){
            if (stmt.elseBranch) execute(*stmt.elseBranch);
        }
    }

    void visit(const While& stmt) const override {
        while(isTruthy(evaluate(*stmt.condition))){
            executeBlock({stmt.body}, environment);
        }
    }

public:
    // Add a getter for globals
    std::shared_ptr<Environment> getGlobals() const { return globals; }
    // Add a public method to execute a block
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment) const {
        auto previous = this->environment;
        this->environment = environment;
        for(const auto& statement : statements){
            execute(*statement);
        }
        this->environment = previous;
    }

private:
    std::shared_ptr<Environment> globals = std::make_shared<Environment>();
    mutable std::shared_ptr<Environment> environment = globals;

    mutable std::ostringstream oss;
    std::string literal_to_string(const literal& value) const {
        if (std::holds_alternative<std::monostate>(value)) return "nil";
        if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
        if (std::holds_alternative<double>(value)) {
            std::ostringstream oss;
            oss << std::get<double>(value);
            return oss.str();
        }
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
        if (std::holds_alternative<std::shared_ptr<LoxCallable>>(value)) {
            auto fn = std::get<std::shared_ptr<LoxCallable>>(value);
            return fn ? fn->toString() : "<fn>";
        }
        return "";
    }

    void checkNumberOperand(const Token& op, std::initializer_list<literal> operands) const {
        for(auto operand : operands){
            if(!std::holds_alternative<double>(operand)){
                throw RuntimeError(op, "Operands must be a number.");
            }
        }
    }

    bool isTruthy(literal object) const {
        if(std::holds_alternative<std::monostate>(object)){
            return false;
        }
        if(std::holds_alternative<bool>(object)) return std::get<bool>(object);
        return true;
    }
};