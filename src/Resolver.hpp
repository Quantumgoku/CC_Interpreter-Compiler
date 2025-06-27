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

enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD
};

enum class ClassType {
    NONE,
    CLASS
};

class Resolver : public ExprVisitorEval, public StmtVisitorEval {
public:
    Resolver(Interpreter& interpreter) : interpreter(interpreter) {}

    lox_literal visit(const Block& stmt) override {
        bool isTopLevel = scopes.empty();
        if (!isTopLevel) beginScope();
        if (!pendingParamsStack.empty()) {
            for (const auto& param : pendingParamsStack.back()) {
                declare(param);
                define(param);
            }
            pendingParamsStack.pop_back();
        }
        auto& stmts = const_cast<std::vector<std::shared_ptr<Stmt>>&>(stmt.statements);
        resolve(stmts);
        if (!isTopLevel) endScope();
        return std::monostate{};
    }

    lox_literal visit(const Variable& expr) override {
        if (!scopes.empty() && scopes.back().count(expr.name.getLexeme()) && scopes.back().at(expr.name.getLexeme()) == false) {
            throw RuntimeError(expr.name, "Cannot read variable '" + expr.name.getLexeme() + "' in its own initializer.");
        }
        resolveLocal(expr, expr.name);
        return std::monostate{};
    }

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
        for (const auto& arg : expr.arguments) {
            resolve(*arg);
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

    lox_literal visit(const This& expr) override {
        if (currentClass == ClassType::NONE) {
            throw RuntimeError(expr.keyword, "Can't use 'this' outside of a class.");
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
        if (stmt.superclass.has_value()) {
            auto ptr = stmt.superclass.value();
            if (ptr) {
                auto var = std::dynamic_pointer_cast<Variable>(ptr);
                if (var && stmt.name.getLexeme() == var->name.getLexeme()) {
                    throw RuntimeError(stmt.name, "A class cannot inherit from itself.");
                }
                resolve(*ptr);
            }
        }
        define(stmt.name);
        beginScope();
        scopes.back()["this"] = true; // 'this' is always defined in class scope
        for( const auto& method : stmt.methods) {
            FunctionType declaration = FunctionType::METHOD;
            if(method->name.getLexeme() == "init") {
                declaration = FunctionType::INITIALIZER;
            }
            resolveFunction(*method, declaration);
        }
        endScope();
        currentClass = enclosingClass;
        return std::monostate{};
    }

    void resolve(std::vector<std::shared_ptr<Stmt>>& statements) {
        // Ensure the global scope is always present
        if (scopes.empty()) {
            beginScope();
        }
        for (auto& statement : statements) {
            resolve(*statement);
        }
        // Do NOT end the global scope here; keep it for the lifetime of the resolver
    }

private:
    Interpreter& interpreter;
    std::vector<std::unordered_map<std::string, bool>> scopes;
    // Use a stack for pendingParams to handle nested functions correctly
    std::vector<std::vector<Token>> pendingParamsStack;
    FunctionType currentFunction = FunctionType::NONE;
    ClassType currentClass = ClassType::NONE;

    void resolveFunction(const Function& function, FunctionType type = FunctionType::FUNCTION) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = type;
        pendingParamsStack.push_back(function.params);
        resolve(*function.body); // body is a shared_ptr<Stmt>
        currentFunction = enclosingFunction;
    }

    void declare(const Token& name) {
        // Only check for redeclaration in local scopes, not global
        if (scopes.empty()) return;
        if (scopes.size() == 1) return; // Allow redeclaration in global scope
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

    void beginScope() {
        scopes.push_back(std::unordered_map<std::string, bool>());
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
            if (it != scopes[i].end()) { // Always resolve to nearest declaration
                //std::cerr << "[Resolver] resolveLocal: " << name.getLexeme() << " expr ptr=" << &expr << " depth=" << (scopes.size() - 1 - i) << std::endl;
                interpreter.resolve(&expr, scopes.size() - 1 - i);
                return;
            }
        }
        // If not found, assume global (do not throw)
    }

    void resolve(Stmt& stmt) {
        stmt.accept(*this);
    }

    void resolve(Expr& expr) {
        expr.accept(*this);
    }
};