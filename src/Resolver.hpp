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

class Resolver : public ExprVisitorEval, public StmtVisitorEval {
public:
    Resolver(Interpreter& interpreter) : interpreter(interpreter) {}

    lox_literal visit(const Block& stmt) const override {
        beginScope();
        if (!pendingParamsStack.empty()) {
            for (const auto& param : pendingParamsStack.back()) {
                declare(param);
                define(param);
            }
            pendingParamsStack.pop_back();
        }
        resolve(stmt.statements);
        endScope();
        return std::monostate{};
    }

    lox_literal visit(const Variable& expr) const override {
        if (!scopes.empty() && scopes.back().count(expr.name.getLexeme()) && scopes.back().at(expr.name.getLexeme()) == false) {
            throw RuntimeError(expr.name, "Cannot read variable '" + expr.name.getLexeme() + "' in its own initializer.");
        }
        resolveLocal(expr, expr.name);
        return std::monostate{};
    }

    lox_literal visit(const Assign& expr) const override {
        resolve(*expr.value);
        resolveLocal(expr, expr.name);
        return std::monostate{};
    }

    lox_literal visit(const Binary& expr) const override {
        resolve(*expr.left);
        resolve(*expr.right);
        return std::monostate{};
    }

    lox_literal visit(const Call& expr) const override {
        resolve(*expr.callee);
        for (const auto& arg : expr.arguments) {
            resolve(*arg);
        }
        return std::monostate{};
    }

    lox_literal visit(const Grouping& expr) const override {
        resolve(*expr.expression);
        return std::monostate{};
    }

    lox_literal visit(const Literal& expr) const override {
        return std::monostate{};
    }

    lox_literal visit(const Logical& expr) const override {
        resolve(*expr.left);
        resolve(*expr.right);
        return std::monostate{};
    }

    lox_literal visit(const Unary& expr) const override {
        resolve(*expr.right);
        return std::monostate{};
    }

    lox_literal visit(const Var& stmt) const override {
        declare(stmt.name);
        if (stmt.initializer != nullptr) {
            resolve(*stmt.initializer);
        }
        define(stmt.name);
        return std::monostate{};
    }

    lox_literal visit(const Function& stmt) const override {
        declare(stmt.name);
        define(stmt.name);
        resolveFunction(stmt);
        return std::monostate{};
    }

    lox_literal visit(const Expression& stmt) const override {
        resolve(*stmt.expression);
        return std::monostate{};
    }

    lox_literal visit(const If& stmt) const override {
        resolve(*stmt.condition);
        resolve(*stmt.thenBranch);
        if (stmt.elseBranch) {
            resolve(*stmt.elseBranch);
        }
        return std::monostate{};
    }

    lox_literal visit(const Print& stmt) const override {
        resolve(*stmt.expression);
        return std::monostate{};
    }

    lox_literal visit(const Return& stmt) const override {
        if (stmt.value != nullptr) {
            resolve(*stmt.value);
        }
        return std::monostate{};
    }

    lox_literal visit(const While& stmt) const override {
        resolve(*stmt.condition);
        resolve(*stmt.body);
        return std::monostate{};
    }

    void resolve(const std::vector<std::shared_ptr<Stmt>>& statements) const {
        for (const auto& statement : statements) {
            resolve(*statement);
        }
    }

private:
    Interpreter& interpreter;
    mutable std::vector<std::unordered_map<std::string, bool>> scopes;
    // Use a stack for pendingParams to handle nested functions correctly
    mutable std::vector<std::vector<Token>> pendingParamsStack;

    void resolveFunction(const Function& function) const {
        pendingParamsStack.push_back(function.params);
        resolve(*function.body); // body is a shared_ptr<Stmt>
    }

    void declare(const Token& name) const {
        if (scopes.empty()) return;
        auto& scope = scopes.back();
        if (scope.find(name.getLexeme()) != scope.end()) {
            throw RuntimeError(name, "Variable '" + name.getLexeme() + "' already declared in this scope.");
        }
        scope[name.getLexeme()] = false;
    }

    void define(const Token& name) const {
        if (scopes.empty()) return;
        scopes.back()[name.getLexeme()] = true;
    }

    void beginScope() const {
        scopes.push_back(std::unordered_map<std::string, bool>());
    }

    void endScope() const {
        if (scopes.empty()) {
            throw RuntimeError(Token(TokenType::IDENTIFIER, "", std::monostate{}, 0), "No scope to end.");
        }
        scopes.pop_back();
    }

    void resolveLocal(const Expr& expr, const Token& name) const {
        for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; --i) {
            auto it = scopes[i].find(name.getLexeme());
            if (it != scopes[i].end()) { // Always resolve to nearest declaration
                interpreter.resolve(std::shared_ptr<Expr>(const_cast<Expr*>(&expr), [](Expr*){}), scopes.size() - 1 - i);
                return;
            }
        }
        // If not found, assume global (do not throw)
    }

    void resolve(const Stmt& stmt) const {
        stmt.accept(*this);
    }

    void resolve(const Expr& expr) const {
        expr.accept(*this);
    }
};