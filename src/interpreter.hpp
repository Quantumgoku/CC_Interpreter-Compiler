#pragma once
#include<sstream>
#include<iomanip>
#include "Expr.hpp"
#include "token.hpp"
#include "Environment.hpp"
#include "RuntimeError.hpp"
#include "LoxCallable.hpp"
#include "LoxClock.hpp"
#include "LoxFunction.hpp"
#include "ReturnException.hpp"
#include "literal.hpp"
#include "Stmt.hpp"
#include<unordered_map>
#include "lox_utils.hpp"
#include "literal_to_string.hpp"
#include "LoxClass.hpp"

class Interpreter : public ExprVisitorEval, public StmtVisitorEval {
public:
    Interpreter() {
        globals->define("clock", std::make_shared<LoxClock>());
        environment = globals;
    }

    lox_literal visit(const Expression& stmt) const override {
        return evaluate(*stmt.expression);
    }

    lox_literal visit(const Print& stmt) const override {
        lox_literal value = evaluate(*stmt.expression);
        std::cout << literal_to_string(value) << std::endl;
        return std::monostate{};
    }

    void execute(Stmt& stmt) const {
        stmt.accept(*this);
    }

    void resolve(const Expr* expr, size_t depth) const {
        locals.emplace(expr, depth);
    }

    lox_literal evaluate(const Expr& expr) const {
        return expr.accept(*this);
    }

    lox_literal visit(const Literal& expr) const override{
        return std::visit([](auto&& arg) -> lox_literal {
            using T = std::decay_t<decltype(arg)>;
            return lox_literal(arg);
        }, expr.value);
    }

    lox_literal visit(const Grouping& expr) const override {
        return evaluate(*expr.expression);
    }

    lox_literal visit(const Variable& expr) const override {
        return lookUpVariable(expr.name, expr);
    }

    lox_literal visit(const Logical& expr) const override {
        lox_literal left = evaluate(*expr.left);
        if(expr.op.getTokenType() == TokenType::OR){
            if(isTruthy(left)) return left;
        } else if(expr.op.getTokenType() == TokenType::AND){
            if(!isTruthy(left)) return left;
        }
        // Only evaluate right if needed (short-circuit)
        return evaluate(*expr.right);
    }

    lox_literal visit(const Var& stmt) const override {
        lox_literal value;
        if (stmt.initializer != nullptr) {
            value = evaluate(*stmt.initializer);
        } else {
            value = std::monostate{};
        }
        environment->define(stmt.name.getLexeme(), value);
        return std::monostate{};
    }

    lox_literal visit(const Binary& expr) const override {
        lox_literal left = evaluate(*expr.left);
        lox_literal right = evaluate(*expr.right);
        switch(expr.op.getTokenType()){
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

    lox_literal visit(const Unary& expr) const override {
        lox_literal right = evaluate(*expr.right);
        switch(expr.op.getTokenType()){
            case TokenType::MINUS:
                checkNumberOperand(expr.op, {right});
                return -(std::get<double>(right));
            case TokenType::BANG:
                return !isTruthy(right);
        }  
        return std::monostate{};
    }

    lox_literal visit(const Assign& expr) const override {
        lox_literal value = evaluate(*expr.value);
        auto it = locals.find(&expr);
        if (it != locals.end()) {
            int distance = it->second;
            environment->assignAt(distance, expr.name, value);
        } else {
            globals->assign(expr.name, value);
        }
        return value;
    }

    lox_literal visit(const Call& expr) const override {
        lox_literal callee = evaluate(*expr.callee);
        std::vector<lox_literal> arguments;

        for(const auto& arg : expr.arguments){
            arguments.push_back(evaluate(*arg));
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

    lox_literal visit(const Get& expr) const override {
        lox_literal object = evaluate(*expr.object);
        if(!std::holds_alternative<std::shared_ptr<LoxInstance>>(object)){
            throw RuntimeError(expr.name, "Only instances have properties.");
        }
        auto instance = std::get<std::shared_ptr<LoxInstance>>(object);
        return instance->get(expr.name);
    }

    lox_literal visit(const Set& expr) const override {
        lox_literal object = evaluate(*expr.object);
        if(!std::holds_alternative<std::shared_ptr<LoxInstance>>(object)){
            throw RuntimeError(expr.name, "Only instances have fields.");
        }
        lox_literal value = evaluate(*expr.value);
        auto instance = std::get<std::shared_ptr<LoxInstance>>(object);
        instance->set(expr.name, value);
        return value;
    }

    lox_literal visit(const This& expr) const override {
        return lookUpVariable(expr.keyword, expr);
    }

    lox_literal visit(const Function& stmt) const override {
        // Use the correct closure for nested functions
        std::shared_ptr<Environment> closureToCapture = closureForNestedFunctions ? closureForNestedFunctions : environment;
        auto function = std::make_shared<LoxFunction>(std::make_shared<Function>(stmt), closureToCapture, false);
        environment->define(stmt.name.getLexeme(), function);
        return std::monostate{};
    }

    lox_literal visit(const Return& stmt) const override {
        lox_literal value = std::monostate{};
        if (stmt.value != nullptr) {
            value = evaluate(*stmt.value);
        }
        throw ReturnException(value);
    }

    lox_literal visit(const Block& stmt) const override {
        // Always create a new environment for a block, even at global scope
        auto newEnv = std::make_shared<Environment>(environment);
        executeBlock(stmt.statements, newEnv);
        return std::monostate{};
    }

    lox_literal visit(const Class& stmt) const override {
        lox_literal supperClass = std::monostate{};
        std::shared_ptr<LoxClass> superClassPtr = nullptr;
        if (stmt.superclass) {
            supperClass = evaluate(**stmt.superclass);
            if (!std::holds_alternative<std::shared_ptr<LoxCallable>>(supperClass)) {
                throw RuntimeError(stmt.name, "Superclass must be a class.");
            }
            superClassPtr = std::dynamic_pointer_cast<LoxClass>(std::get<std::shared_ptr<LoxCallable>>(supperClass));
            if (!superClassPtr) {
                throw RuntimeError(stmt.name, "Superclass must be a class.");
            }
        }
        environment->define(stmt.name.getLexeme(), std::monostate{});
        std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
        for(const auto& method : stmt.methods) {
            std::shared_ptr<LoxFunction> function;
            // Set closureForNestedFunctions to the method closure while defining the method
            auto prevClosure = closureForNestedFunctions;
            closureForNestedFunctions = environment;
            function = std::make_shared<LoxFunction>(method, environment, method->name.getLexeme() == "init");
            closureForNestedFunctions = prevClosure;
            methods[method->name.getLexeme()] = function;
        }
        std::shared_ptr<LoxCallable> klass = LoxClass::create(stmt.name.getLexeme(), std::weak_ptr<LoxClass>(superClassPtr), methods);
        environment->assign(stmt.name, klass);
        return std::monostate{};
    }

    lox_literal visit(const If& stmt) const override {
        lox_literal condition = evaluate(*stmt.condition);
        if (isTruthy(condition)) {
            if (stmt.thenBranch) execute(*stmt.thenBranch);
        } else if (stmt.elseBranch != nullptr) {
            if (stmt.elseBranch) execute(*stmt.elseBranch);
        }
        return std::monostate{};
    }

    lox_literal visit(const While& stmt) const override {
        while (isTruthy(evaluate(*stmt.condition))) {
            executeBlock({stmt.body}, environment);
        }
        return std::monostate{};
    }

public:
    // Add a getter for globals
    std::shared_ptr<Environment> getGlobals() const { return globals; }
    // Add a public method to execute a block
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment) const {
        auto previous = this->environment;
        this->environment = environment;
        try {
            for(size_t i = 0; i < statements.size(); ++i){
                execute(*statements[i]);
            }
        } catch (const ReturnException&) {
            this->environment = previous;
            throw;
        }
        this->environment = previous;
    }

private:
    std::shared_ptr<Environment> globals = std::make_shared<Environment>();
    mutable std::shared_ptr<Environment> environment = globals;
    mutable std::shared_ptr<Environment> closureForNestedFunctions = nullptr;
    mutable std::unordered_map<const Expr*, size_t> locals;

    mutable std::ostringstream oss;

    lox_literal lookUpVariable(const Token& name, const Expr& expr) const {
        auto it = locals.find(&expr);
        if (it != locals.end()) {
            int distance = it->second;
            return environment->getAt(distance, name.getLexeme());
        } else {
            return globals->getValue(name);
        }
        // Defensive: if somehow neither branch returns, throw a clear error
        throw RuntimeError(name, "Variable resolution failed for '" + name.getLexeme() + "'.");
    }
};