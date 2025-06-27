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
#include <iostream> // For std::cout and std::cerr

class Interpreter : public ExprVisitorEval, public StmtVisitorEval {
public:
    Interpreter() {
        globals->define("clock", std::make_shared<LoxClock>());
        environment = globals;
    }

    lox_literal visit(const Expression& stmt) {
        return evaluate(*stmt.expression);
    }

    lox_literal visit(const Print& stmt) override {
        lox_literal value = evaluate(*stmt.expression);
        std::cout << literal_to_string(value) << std::endl;
        return std::monostate{};
    }

    void execute(Stmt& stmt) {
        stmt.accept(*this);
    }

    void resolve(const Expr* expr, int depth) {
        locals.emplace(expr, depth);
    }

    lox_literal evaluate(const Expr& expr) {
        return expr.accept(*this);
    }

    lox_literal visit(const Literal& expr) override{
        return std::visit([](auto&& arg) -> lox_literal {
            using T = std::decay_t<decltype(arg)>;
            return lox_literal(arg);
        }, expr.value);
    }

    lox_literal visit(const Grouping& expr) override {
        return evaluate(*expr.expression);
    }

    lox_literal visit(const Variable& expr) override {
        return lookUpVariable(expr.name, expr);
    }

    lox_literal visit(const Logical& expr) override {
        lox_literal left = evaluate(*expr.left);
        if(expr.op.getTokenType() == TokenType::OR){
            if(isTruthy(left)) return left;
        } else if(expr.op.getTokenType() == TokenType::AND){
            if(!isTruthy(left)) return left;
        }
        // Only evaluate right if needed (short-circuit)
        return evaluate(*expr.right);
    }

    lox_literal visit(const Var& stmt) override {
        lox_literal value;
        if (stmt.initializer != nullptr) {
            value = evaluate(*stmt.initializer);
        } else {
            value = std::monostate{};
        }
        environment->define(stmt.name.getLexeme(), value);
        return std::monostate{};
    }

    lox_literal visit(const Binary& expr) override {
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

    lox_literal visit(const Unary& expr) override {
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

    lox_literal visit(const Assign& expr) override {
        lox_literal value = evaluate(*expr.value);
        auto it = locals.find(&expr);
        if (it != locals.end()) {
            int distance = static_cast<int>(it->second);
            environment->assignAt(distance, expr.name, value);
        } else {
            globals->assign(expr.name, value);
        }
        return value;
    }

    lox_literal visit(const Call& expr) override {
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

        lox_literal result = (*functionPtr)->call(*this, arguments);
        // If the result is a function and the callee is a method (has 'this'), bind the returned function to the same instance
        if (std::holds_alternative<std::shared_ptr<LoxCallable>>(result)) {
            auto returnedFunc = std::get<std::shared_ptr<LoxCallable>>(result);
            auto returnedLoxFunc = std::dynamic_pointer_cast<LoxFunction>(returnedFunc);
            auto calleeLoxFunc = std::dynamic_pointer_cast<LoxFunction>(*functionPtr);
            if (returnedLoxFunc && calleeLoxFunc && calleeLoxFunc->getClosure() && calleeLoxFunc->getClosure()->has("this")) {
                auto instance = std::get<std::shared_ptr<LoxInstance>>(calleeLoxFunc->getClosure()->getValue(Token(TokenType::IDENTIFIER, "this", std::monostate{}, 0)));
                return returnedLoxFunc->bind(instance);
            }
        }
        return result;
    }

    lox_literal visit(const Get& expr) override {
        lox_literal object = evaluate(*expr.object);
        if(!std::holds_alternative<std::shared_ptr<LoxInstance>>(object)){
            throw RuntimeError(expr.name, "Only instances have properties.");
        }
        auto instance = std::get<std::shared_ptr<LoxInstance>>(object);
        lox_literal value = instance->get(expr.name);
        // Do NOT rebind here! LoxInstance::get already handles correct rebinding semantics.
        return value;
    }

    lox_literal visit(const Set& expr) override {
        lox_literal object = evaluate(*expr.object);
        if(!std::holds_alternative<std::shared_ptr<LoxInstance>>(object)){
            throw RuntimeError(expr.name, "Only instances have fields.");
        }
        lox_literal value = evaluate(*expr.value);
        auto instance = std::get<std::shared_ptr<LoxInstance>>(object);
        instance->set(expr.name, value);
        return value;
    }

    lox_literal visit(const This& expr) override {
        return lookUpVariable(expr.keyword, expr);
    }

    lox_literal visit(const Function& stmt) override {
        // For nested functions in methods, we need to capture the environment that contains 'this'
        std::shared_ptr<Environment> closureToCapture = closureForNestedFunctions ? closureForNestedFunctions : environment;
        
        // If we're in a method and the current environment doesn't have 'this', 
        // check if the enclosing environment has it
        if (closureToCapture && !closureToCapture->has("this") && closureToCapture->getEnclosing() && closureToCapture->getEnclosing()->has("this")) {
            closureToCapture = closureToCapture->getEnclosing();
        }
        
        // Ensure closureToCapture is not null
        if (!closureToCapture) {
            throw RuntimeError(stmt.name, "Internal error: closure environment null when defining function.");
        }
        auto function = std::make_shared<LoxFunction>(std::make_shared<Function>(stmt), closureToCapture, false);
        environment->define(stmt.name.getLexeme(), function);
        return std::monostate{};
    }

    lox_literal visit(const Return& stmt) override {
        lox_literal value = std::monostate{};
        if (stmt.value != nullptr) {
            value = evaluate(*stmt.value);
        }
        throw ReturnException(value);
    }

    lox_literal visit(const Block& stmt) override {
        // Capture the current environment in a local shared_ptr to ensure it stays alive
        std::shared_ptr<Environment> parentEnv = environment;
        auto newEnv = std::make_shared<Environment>(parentEnv);

        // Only set closureForNestedFunctions if it is not already set (i.e., outermost block of a function/method)
        bool shouldSetClosure = !closureForNestedFunctions;
        auto prevClosure = closureForNestedFunctions;
        if (shouldSetClosure) {
            closureForNestedFunctions = newEnv;
        }

        executeBlock(stmt.statements, newEnv);

        if (shouldSetClosure) {
            closureForNestedFunctions = prevClosure;
        }

        return std::monostate{};
    }

    lox_literal visit(const Class& stmt) override {
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
        
        // Create a placeholder class to define the name first
        std::unordered_map<std::string, std::shared_ptr<LoxFunction>> emptyMethods;
        auto placeholder = LoxClass::create(stmt.name.getLexeme(), std::weak_ptr<LoxClass>(superClassPtr), emptyMethods);
        environment->define(stmt.name.getLexeme(), placeholder);
        
        std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
        for(const auto& method : stmt.methods) {
            std::shared_ptr<LoxFunction> function;
            // Defensive: ensure environment is a valid shared_ptr
            auto prevClosure = closureForNestedFunctions;
            closureForNestedFunctions = environment;
            if (!environment) {
                throw RuntimeError(method->name, "Internal error: method closure environment expired.");
            }
            function = std::make_shared<LoxFunction>(method, environment, method->name.getLexeme() == "init");
            closureForNestedFunctions = prevClosure;
            methods[method->name.getLexeme()] = function;
        }
        
        // Replace the placeholder with the real class
        std::shared_ptr<LoxCallable> klass = LoxClass::create(stmt.name.getLexeme(), std::weak_ptr<LoxClass>(superClassPtr), methods);
        environment->define(stmt.name.getLexeme(), klass);
        return std::monostate{};
    }

    lox_literal visit(const If& stmt) override {
        lox_literal condition = evaluate(*stmt.condition);
        if (isTruthy(condition)) {
            if (stmt.thenBranch) execute(*stmt.thenBranch);
        } else if (stmt.elseBranch != nullptr) {
            if (stmt.elseBranch) execute(*stmt.elseBranch);
        }
        return std::monostate{};
    }

    lox_literal visit(const While& stmt) override {
        while (isTruthy(evaluate(*stmt.condition))) {
            execute(*stmt.body);
        }
        return std::monostate{};
    }

    lox_literal lookUpVariable(const Token& name, const Expr& expr) const {
        auto it = locals.find(&expr);
        if (it != locals.end()) {
            int distance = it->second;
            try {
                return environment->getAt(distance, name.getLexeme());
            } catch (const RuntimeError&) {
                // If resolved distance fails, try searching the environment chain manually
                // This handles cases where resolver distance calculations don't match runtime structure
                auto current = environment;
                while (current) {
                    if (current->has(name.getLexeme())) {
                        return current->getValue(name);
                    }
                    current = current->getEnclosing();
                }
                throw RuntimeError(name, "Undefined variable '" + name.getLexeme() + "'.");
            }
        } else {
            return globals->getValue(name);
        }
    }

    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> newEnvironment) {
        // Save the current environment
        auto previousEnvironment = this->environment;

        // Link the new environment to the current one
        if (previousEnvironment && !newEnvironment->getEnclosing()) {
            newEnvironment->setEnclosing(previousEnvironment);
        }

        // Update the interpreter's environment to the new one
        this->environment = newEnvironment;

        try {
            // Execute each statement in the block
            for (size_t i = 0; i < statements.size(); ++i) {
                execute(*statements[i]);
            }
        } catch (const ReturnException&) {
            // Restore the previous environment before propagating the exception
            this->environment = previousEnvironment;
            throw;
        }

        // Restore the previous environment after block execution
        this->environment = previousEnvironment;
    }

    mutable std::shared_ptr<Environment> closureForNestedFunctions = nullptr;

    int getEnvironmentDepth() const {
        int depth = 0;
        std::shared_ptr<Environment> env = environment;
        while (env) {
            depth++;
            env = env->getEnclosing();
        }
        return depth;
    }

    void printEnvironmentChain() const {
        int depth = 0;
        auto env = environment;
        while (env) {
            depth++;
            env = env->getEnclosing();
        }
    }

    // Allow LoxFunction to temporarily set the environment
    std::shared_ptr<Environment> getCurrentEnvironment() const {
        return environment;
    }
    
    void setCurrentEnvironment(std::shared_ptr<Environment> env) {
        environment = env;
    }

private:
    std::shared_ptr<Environment> globals = std::make_shared<Environment>();
    mutable std::shared_ptr<Environment> environment = globals;
    
    mutable std::unordered_map<const Expr*, int> locals;

    mutable std::ostringstream oss;
};