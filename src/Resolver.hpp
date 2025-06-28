#pragma once

#include "interpreter.hpp"
#include "token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "parser.hpp"
#include "literal.hpp"
#include "Environment.hpp"
#include "RuntimeError.hpp"
#include <vector>
#include <unordered_map>
#include <iostream>

/** Track the type of function currently being resolved. */
enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD
};

/** Track the type of class currently being resolved. */
enum class ClassType {
    NONE,
    CLASS,
    SUBCLASS
};

/**
 * The Resolver performs static analysis on the AST to resolve variable 
 * bindings and detect scope-related errors. It calculates the distance
 * from each variable use to its declaration for efficient lookup.
 */
class Resolver : public ExprVisitorEval, public StmtVisitorEval {
public:
    Resolver(Interpreter& interpreter) : interpreter(interpreter) {}
    
    /** Start a new lexical scope. */
    void beginScope() {
        scopes.emplace_back();
    }

    /** Resolve a block statement by creating a new scope. */
    lox_literal visit(const Block& stmt) override {
        beginScope();
        auto& stmts = const_cast<std::vector<std::shared_ptr<Stmt>>&>(stmt.statements);
        resolve(stmts);
        endScope();
        return std::monostate{};
    }

    /** Resolve variable access, checking for self-initialization. */
    lox_literal visit(const Variable& expr) override {
        if (!scopes.empty() && scopes.back().count(expr.name.getLexeme()) && scopes.back().at(expr.name.getLexeme()) == false) {
            throw RuntimeError(expr.name, "Cannot read variable '" + expr.name.getLexeme() + "' in its own initializer.");
        }
        resolveLocal(expr, expr.name);
        return std::monostate{};
    }

    /** Resolve variable assignment. */
    lox_literal visit(const Assign& expr) override {
        resolve(*expr.value);
        resolveLocal(expr, expr.name);
        return std::monostate{};
    }

    lox_literal visit(const Binary& expr) override {
        resolve(*expr.left);
        resolve(*expr.right);
        return std::monostate{};
    }

    lox_literal visit(const Call& expr) override {
        resolve(*expr.callee);
        for (const auto& argument : expr.arguments) {
            resolve(*argument);
        }
        return std::monostate{};
    }

    lox_literal visit(const Grouping& expr) override {
        resolve(*expr.expression);
        return std::monostate{};
    }

    lox_literal visit(const Literal& expr) override {
        return std::monostate{};
    }

    lox_literal visit(const Logical& expr) override {
        resolve(*expr.left);
        resolve(*expr.right);
        return std::monostate{};
    }

    lox_literal visit(const Unary& expr) override {
        resolve(*expr.right);
        return std::monostate{};
    }

    lox_literal visit(const Get& expr) override {
        resolve(*expr.object);
        return std::monostate{};
    }

    lox_literal visit(const Set& expr) override {
        resolve(*expr.value);
        resolve(*expr.object);
        return std::monostate{};
    }

    /** Resolve 'this' keyword, ensuring it's used inside a class. */
    lox_literal visit(const This& expr) override {
        if (currentClass == ClassType::NONE) {
            throw RuntimeError(expr.keyword, "Can't use 'this' outside of a class.");
        }
        resolveLocal(expr, expr.keyword);
        return std::monostate{};
    }

    /** Resolve 'super' keyword, ensuring it's used in a subclass. */
    lox_literal visit(const Super& expr) override {
        if (currentClass == ClassType::NONE) {
            throw RuntimeError(expr.keyword, "Can't use 'super' outside of a class.");
        } else if (currentClass != ClassType::SUBCLASS) {
            throw RuntimeError(expr.keyword, "Can't use 'super' in a class with no superclass.");
        }
        resolveLocal(expr, expr.keyword);
        return std::monostate{};
    }

    lox_literal visit(const Var& stmt) override {
        declare(stmt.name);
        if (stmt.initializer != nullptr) {
            resolve(*stmt.initializer);
        }
        define(stmt.name);
        return std::monostate{};
    }

    lox_literal visit(const Function& stmt) override {
        declare(stmt.name);
        define(stmt.name);
        resolveFunction(stmt, FunctionType::FUNCTION);
        return std::monostate{};
    }

    lox_literal visit(const Expression& stmt) override {
        resolve(*stmt.expression);
        return std::monostate{};
    }

    lox_literal visit(const If& stmt) override {
        resolve(*stmt.condition);
        resolve(*stmt.thenBranch);
        if (stmt.elseBranch) {
            resolve(*stmt.elseBranch);
        }
        return std::monostate{};
    }

    lox_literal visit(const Print& stmt) override {
        resolve(*stmt.expression);
        return std::monostate{};
    }

    lox_literal visit(const Return& stmt) override {
        if (currentFunction == FunctionType::NONE) {
            throw RuntimeError(stmt.keyword, "Can't return from top-level code.");
        }
        if (stmt.value != nullptr) {
            if(currentFunction == FunctionType::INITIALIZER) {
                throw RuntimeError(stmt.keyword, "Can't return a value from an initializer.");
            }
            resolve(*stmt.value);
        }
        return std::monostate{};
    }

    lox_literal visit(const While& stmt) override {
        resolve(*stmt.condition);
        resolve(*stmt.body);
        return std::monostate{};
    }

    lox_literal visit(const Class& stmt) override {
        ClassType enclosingClass = currentClass;
        currentClass = ClassType::CLASS;
        
        declare(stmt.name);
        define(stmt.name);
        
        if (stmt.superclass.has_value()) {
            currentClass = ClassType::SUBCLASS;
            auto ptr = stmt.superclass.value();
            if (ptr) {
                auto var = std::dynamic_pointer_cast<Variable>(ptr);
                if (var && stmt.name.getLexeme() == var->name.getLexeme()) {
                    throw RuntimeError(stmt.name, "A class cannot inherit from itself.");
                }
                resolve(*ptr);
            }
            
            beginScope();
            scopes.back()["super"] = true;
        }
        
        beginScope();
        scopes.back()["this"] = true;
        
        for( const auto& method : stmt.methods) {
            FunctionType declaration = FunctionType::METHOD;
            if(method->name.getLexeme() == "init") {
                declaration = FunctionType::INITIALIZER;
            }
            resolveFunction(*method, declaration);
        }
        
        endScope();
        
        if (stmt.superclass.has_value()) {
            endScope();
        }
        
        currentClass = enclosingClass;
        return std::monostate{};
    }

    void resolve(std::vector<std::shared_ptr<Stmt>>& statements) {
        for (auto& statement : statements) {
            resolve(*statement);
        }
    }

    void resolve(Stmt& stmt) {
        stmt.accept(*this);
    }

    void resolve(Expr& expr) {
        expr.accept(*this);
    }

private:
    Interpreter& interpreter;
    std::vector<std::unordered_map<std::string, bool>> scopes;
    FunctionType currentFunction = FunctionType::NONE;
    ClassType currentClass = ClassType::NONE;

    void resolveFunction(const Function& function, FunctionType type = FunctionType::FUNCTION) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = type;
        beginScope(); // Create scope for this function
        
        // Add 'this' for methods
        if (type == FunctionType::METHOD || type == FunctionType::INITIALIZER) {
            scopes.back()["this"] = true;
        }
        
        // Add parameters to this function's scope
        for (const auto& param : function.params) {
            declare(param);
            define(param);
        }
        
        // Resolve the function body statements directly (not as a block)
        if (auto block = std::dynamic_pointer_cast<Block>(function.body)) {
            // For function body blocks, resolve statements directly without creating a new scope
            auto& stmts = const_cast<std::vector<std::shared_ptr<Stmt>>&>(block->statements);
            resolve(stmts);
        } else {
            resolve(*function.body);
        }
        
        endScope(); // Pop this function's scope
        currentFunction = enclosingFunction;
    }

    void declare(const Token& name) {
        // Only check for redeclaration in local scopes, not global
        if (scopes.empty()) return;
        auto& scope = scopes.back();
        if (scope.find(name.getLexeme()) != scope.end()) {
            throw RuntimeError(name, "Variable '" + name.getLexeme() + "' already declared in this scope.");
        }
        scope[name.getLexeme()] = false;
    }

    void define(const Token& name) {
        if (scopes.empty()) return;
        scopes.back()[name.getLexeme()] = true;
    }

    void endScope() {
        if (scopes.empty()) {
            throw RuntimeError(Token(TokenType::IDENTIFIER, "", std::monostate{}, 0), "No scope to end.");
        }
        scopes.pop_back();
    }

    void resolveLocal(const Expr& expr, const Token& name) {
        for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
            auto it = scopes[i].find(name.getLexeme());
            if (it != scopes[i].end()) {
                int distance = static_cast<int>(scopes.size() - 1 - i);
                // Always record distance for variables found in local scopes
                interpreter.resolve(&expr, distance);
                return;
            }
        }
        // If not found in any local scope, assume it's global - don't throw error
    }
};