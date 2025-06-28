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
#include <iostream>

/**
 * The Interpreter evaluates Lox expressions and executes statements.
 * It implements the visitor pattern for both expressions and statements,
 * managing execution environments and variable resolution.
 */
class Interpreter : public ExprVisitorEval, public StmtVisitorEval {
public:
    /**
     * Initialize the interpreter with global environment and built-in functions.
     */
    Interpreter() {
        globals->define("clock", std::make_shared<LoxClock>());
        environment = globals;
    }

    /** Execute an expression statement and return the result. */
    lox_literal visit(const Expression& stmt) {
        return evaluate(*stmt.expression);
    }

    /** Execute a print statement, outputting the value to stdout. */
    lox_literal visit(const Print& stmt) override {
        lox_literal value = evaluate(*stmt.expression);
        std::cout << literal_to_string(value) << std::endl;
        return std::monostate{};
    }

    /** Execute any statement using the visitor pattern. */
    void execute(Stmt& stmt) {
        stmt.accept(*this);
    }

    /** Store the resolved distance for a variable lookup. Called by the Resolver. */
    void resolve(const Expr* expr, int depth) {
        locals.emplace(expr, depth);
    }

    /** Evaluate any expression using the visitor pattern. */
    lox_literal evaluate(const Expr& expr) {
        return expr.accept(*this);
    }

    /** Return the literal value directly. */
    lox_literal visit(const Literal& expr) override{
        return std::visit([](auto&& arg) -> lox_literal {
            using T = std::decay_t<decltype(arg)>;
            return lox_literal(arg);
        }, expr.value);
    }

    /** Evaluate the grouped expression. */
    lox_literal visit(const Grouping& expr) override {
        return evaluate(*expr.expression);
    }

    /** Look up a variable's value using resolved distance or global scope. */
    lox_literal visit(const Variable& expr) override {
        return lookUpVariable(expr.name, expr);
    }

    /** Handle logical operators (AND, OR) with short-circuit evaluation. */
    lox_literal visit(const Logical& expr) override {
        lox_literal left = evaluate(*expr.left);
        if(expr.op.getTokenType() == TokenType::OR){
            if(isTruthy(left)) return left;
        } else if(expr.op.getTokenType() == TokenType::AND){
            if(!isTruthy(left)) return left;
        }
        return evaluate(*expr.right);
    }

    /** Declare a variable in the current environment. */
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

    /** Handle binary operators (+, -, *, /, ==, !=, <, >, <=, >=). */
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

    /** Handle unary operators (-, !). */
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

    /** Assign a value to a variable, using resolved distance if available. */
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

    /** 
     * Execute a function or method call. Also handles special case where 
     * a method returns a closure that should be bound to the same instance.
     */
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
        
        // Special handling for closures returned from methods:
        // If a bound method returns a function, bind that function to the same instance.
        // This ensures that closures using 'super' or 'this' work correctly.
        if (std::holds_alternative<std::shared_ptr<LoxCallable>>(result)) {
            auto returnedFunc = std::get<std::shared_ptr<LoxCallable>>(result);
            auto returnedLoxFunc = std::dynamic_pointer_cast<LoxFunction>(returnedFunc);
            auto calleeLoxFunc = std::dynamic_pointer_cast<LoxFunction>(*functionPtr);
            if (returnedLoxFunc && calleeLoxFunc && calleeLoxFunc->getBoundInstance()) {
                return returnedLoxFunc->bind(calleeLoxFunc->getBoundInstance());
            }
        }
        return result;
    }

    /** Get a property from an instance (instance.property). */
    lox_literal visit(const Get& expr) override {
        lox_literal object = evaluate(*expr.object);
        if(!std::holds_alternative<std::shared_ptr<LoxInstance>>(object)){
            throw RuntimeError(expr.name, "Only instances have properties.");
        }
        auto instance = std::get<std::shared_ptr<LoxInstance>>(object);
        lox_literal value = instance->get(expr.name);
        return value;
    }

    /** Set a property on an instance (instance.property = value). */
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

    /** Look up 'this' reference using resolved distance. */
    lox_literal visit(const This& expr) override {
        return lookUpVariable(expr.keyword, expr);
    }

    /** 
     * Handle 'super.method' calls. Finds the superclass method and binds it 
     * to the current instance. Works correctly in closures by looking for 
     * 'this' in the execution environment.
     */
    lox_literal visit(const Super& expr) override {
        int distance = locals.at(&expr);
        auto superclass = environment->getAt(distance, "super");
        
        // Look for 'this' in the execution environment (distance 0)
        // This works for both direct method calls and closures
        auto instance = environment->getAt(0, "this");
        
        auto superclassPtr = std::get<std::shared_ptr<LoxCallable>>(superclass);
        auto loxClass = std::dynamic_pointer_cast<LoxClass>(superclassPtr);
        if (!loxClass) {
            throw RuntimeError(expr.method, "Superclass is not a class.");
        }
        
        auto method = loxClass->findMethod(expr.method.getLexeme());
        if (!method) {
            throw RuntimeError(expr.method, "Undefined property '" + expr.method.getLexeme() + "'.");
        }
        
        auto instancePtr = std::get<std::shared_ptr<LoxInstance>>(instance);
        return method->bind(instancePtr);
    }

    /** Create a function and store it in the current environment. */
    lox_literal visit(const Function& stmt) override {
        auto function = std::make_shared<LoxFunction>(std::make_shared<Function>(stmt), environment, false);
        environment->define(stmt.name.getLexeme(), function);
        return std::monostate{};
    }

    /** Handle return statements by throwing a special exception. */
    lox_literal visit(const Return& stmt) override {
        lox_literal value = std::monostate{};
        if (stmt.value != nullptr) {
            value = evaluate(*stmt.value);
        }
        throw ReturnException(value);
    }

    /** Execute a block with a new environment scope. */
    lox_literal visit(const Block& stmt) override {
        auto newEnv = std::make_shared<Environment>(environment);
        executeBlock(stmt.statements, newEnv);
        return std::monostate{};
    }

    /**
     * Define a class with inheritance support. Creates proper environment 
     * chains for 'super' and 'this' binding in methods.
     */
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

        environment->define(stmt.name.getLexeme(), std::monostate{});
        
        std::shared_ptr<Environment> classEnvironment = environment;
        
        // Create environment for 'super' if there's a superclass
        if (stmt.superclass) {
            environment = std::make_shared<Environment>(classEnvironment);
            environment->define("super", superClassPtr);
        }
        
        // Create environment for 'this' - methods will capture this environment
        std::shared_ptr<Environment> methodClosureEnv = std::make_shared<Environment>(environment);
        methodClosureEnv->define("this", std::monostate{});
        
        std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
        for(const auto& method : stmt.methods) {
            auto function = std::make_shared<LoxFunction>(method, methodClosureEnv, method->name.getLexeme() == "init");
            methods[method->name.getLexeme()] = function;
        }
        
        environment = classEnvironment;
        
        std::shared_ptr<LoxCallable> klass = LoxClass::create(stmt.name.getLexeme(), std::weak_ptr<LoxClass>(superClassPtr), methods);
        environment->define(stmt.name.getLexeme(), klass);
        return std::monostate{};
    }

    /** Execute if statement with optional else branch. */
    lox_literal visit(const If& stmt) override {
        lox_literal condition = evaluate(*stmt.condition);
        if (isTruthy(condition)) {
            if (stmt.thenBranch) execute(*stmt.thenBranch);
        } else if (stmt.elseBranch != nullptr) {
            if (stmt.elseBranch) execute(*stmt.elseBranch);
        }
        return std::monostate{};
    }

    /** Execute while loop. */
    lox_literal visit(const While& stmt) override {
        while (isTruthy(evaluate(*stmt.condition))) {
            execute(*stmt.body);
        }
        return std::monostate{};
    }

    /** 
     * Look up a variable by name, using resolved distance if available,
     * otherwise searching in global scope.
     */
    lox_literal lookUpVariable(const Token& name, const Expr& expr) const {
        auto it = locals.find(&expr);
        if (it != locals.end()) {
            int distance = it->second;
            return environment->getAt(distance, name.getLexeme());
        } else {
            return globals->getValue(name);
        }
    }

    /** 
     * Execute a block of statements in a new environment scope.
     * Properly handles exceptions and restores the previous environment.
     */
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> newEnvironment) {
        auto previousEnvironment = this->environment;

        if (previousEnvironment && !newEnvironment->getEnclosing()) {
            newEnvironment->setEnclosing(previousEnvironment);
        }

        this->environment = newEnvironment;

        try {
            for (size_t i = 0; i < statements.size(); ++i) {
                execute(*statements[i]);
            }
        } catch (const ReturnException&) {
            this->environment = previousEnvironment;
            throw;
        }

        this->environment = previousEnvironment;
    }

    /** Get the depth of the current environment chain (for debugging). */
    int getEnvironmentDepth() const {
        int depth = 0;
        std::shared_ptr<Environment> env = environment;
        while (env) {
            depth++;
            env = env->getEnclosing();
        }
        return depth;
    }

    /** Print the environment chain (for debugging). */
    void printEnvironmentChain() const {
        int depth = 0;
        auto env = environment;
        while (env) {
            depth++;
            env = env->getEnclosing();
        }
    }

    /** Get the current environment (used by LoxFunction). */
    std::shared_ptr<Environment> getCurrentEnvironment() const {
        return environment;
    }
    
    /** Set the current environment (used by LoxFunction). */
    void setCurrentEnvironment(std::shared_ptr<Environment> env) {
        environment = env;
    }

private:
    /** Global environment containing built-in functions. */
    std::shared_ptr<Environment> globals = std::make_shared<Environment>();
    /** Current execution environment. */
    mutable std::shared_ptr<Environment> environment = globals;
    /** Map of expressions to their resolved variable distances. */
    mutable std::unordered_map<const Expr*, int> locals;
    /** String stream for output formatting. */
    mutable std::ostringstream oss;
};