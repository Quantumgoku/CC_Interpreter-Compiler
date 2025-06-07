#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include "token.hpp"

#include "Expr.hpp"

class Stmt;
class StmtVisitorPrint;

class StmtVisitorEval;

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(const StmtVisitorPrint& visitor) const = 0;
    virtual literal accept(const StmtVisitorEval& visitor) const = 0;
};

// Forward declarations
struct Expression;
struct Print;
struct Var;

class StmtVisitorPrint {
public:
    virtual void visit(const Expression& stmt) const = 0;
    virtual void visit(const Print& stmt) const = 0;
    virtual void visit(const Var& stmt) const = 0;
    virtual ~StmtVisitorPrint() = default;
};

class StmtVisitorEval {
public:
    virtual literal visit(const Expression& stmt) const = 0;
    virtual literal visit(const Print& stmt) const = 0;
    virtual literal visit(const Var& stmt) const = 0;
    virtual ~StmtVisitorEval() = default;
};

class Expression : public Stmt {
public:
    Expression(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    literal accept(const StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> expression;
};

class Print : public Stmt {
public:
    Print(std::shared_ptr<Expr> expression) : expression(expression) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    literal accept(const StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    std::shared_ptr<Expr> expression;
};

class Var : public Stmt {
public:
    Var(Token name, std::shared_ptr<Expr> initializer) : name(name), initializer(initializer) {}
    void accept(const StmtVisitorPrint& visitor) const override { visitor.visit(*this); }
    literal accept(const StmtVisitorEval& visitor) const override { return visitor.visit(*this); }
    Token name;
    std::shared_ptr<Expr> initializer;
};

using StmtPtr = std::shared_ptr<Stmt>;
using StmtList = std::vector<StmtPtr>;
