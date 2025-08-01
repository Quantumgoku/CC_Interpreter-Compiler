#pragma once

#include <vector>
#include "token.hpp"
#include "literal.hpp"
#include "Expr.hpp"

class Stmt;
class StmtVisitorPrint;
class StmtVisitorEval;

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(const StmtVisitorPrint& visitor) const = 0;
    virtual lox_literal accept(StmtVisitorEval& visitor) const = 0;
};

// Forward declarations
class Block;
class Class;
class Expression;
class Function;
class If;
class Print;
class Return;
class Var;
class While;

class StmtVisitorPrint {
public:
    virtual void visit(const Block& stmt) const = 0;
    virtual void visit(const Class& stmt) const = 0;
    virtual void visit(const Expression& stmt) const = 0;
    virtual void visit(const Function& stmt) const = 0;
    virtual void visit(const If& stmt) const = 0;
    virtual void visit(const Print& stmt) const = 0;
    virtual void visit(const Return& stmt) const = 0;
    virtual void visit(const Var& stmt) const = 0;
    virtual void visit(const While& stmt) const = 0;
    virtual ~StmtVisitorPrint() = default;
};

class StmtVisitorEval {
public:
    virtual lox_literal visit(const Block& stmt) = 0;
    virtual lox_literal visit(const Class& stmt) = 0;
    virtual lox_literal visit(const Expression& stmt) = 0;
    virtual lox_literal visit(const Function& stmt) = 0;
    virtual lox_literal visit(const If& stmt) = 0;
    virtual lox_literal visit(const Print& stmt) = 0;
    virtual lox_literal visit(const Return& stmt) = 0;
    virtual lox_literal visit(const Var& stmt) = 0;
    virtual lox_literal visit(const While& stmt) = 0;
    virtual ~StmtVisitorEval() = default;
};

class Block : public Stmt {
public:
    Block(std::vector<std::shared_ptr<Stmt>> statements) : statements(statements) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::vector<std::shared_ptr<Stmt>> statements;
};

class Class : public Stmt {
public:
    Class(Token name, std::optional<std::shared_ptr<Expr>> superclass, std::vector<std::shared_ptr<Function>> methods) : name(name), superclass(superclass), methods(methods) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token name;
    std::optional<std::shared_ptr<Expr>> superclass;
    std::vector<std::shared_ptr<Function>> methods;
};

class Expression : public Stmt {
public:
    Expression(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> expression;
};

class Function : public Stmt {
public:
    Function(Token name, std::vector<Token> params, std::shared_ptr<Stmt> body) : name(name), params(params), body(body) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token name;
    std::vector<Token> params;
    std::shared_ptr<Stmt> body;
};

class If : public Stmt {
public:
    If(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch) : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch;
};

class Print : public Stmt {
public:
    Print(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> expression;
};

class Return : public Stmt {
public:
    Return(Token keyword, std::shared_ptr<Expr> value) : keyword(keyword), value(value) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token keyword;
    std::shared_ptr<Expr> value;
};

class Var : public Stmt {
public:
    Var(Token name, std::shared_ptr<Expr> initializer) : name(name), initializer(initializer) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token name;
    std::shared_ptr<Expr> initializer;
};

class While : public Stmt {
public:
    While(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body) : condition(condition), body(body) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    lox_literal accept(StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
};

// All AST node fields referencing other nodes use std::shared_ptr<T>.
// All containers (vectors) and optionals use shared_ptr for AST nodes.

using StmtPtr = std::shared_ptr<Stmt>;
using StmtList = std::vector<StmtPtr>;
