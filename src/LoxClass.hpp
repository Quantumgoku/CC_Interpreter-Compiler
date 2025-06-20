#pragma once
#include "token.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "literal.hpp"
#include<unordered_map>
#include "LoxCallable.hpp"
#include "LoxInstance.hpp" 

class LoxInstance; 

class LoxClass : public LoxCallable, public std::enable_shared_from_this<LoxClass> {
public:
    LoxClass(const std::string& name, const std::unordered_map<std::string, std::shared_ptr<LoxFunction>>& methods)
        : name(name), methods(methods) {}
    std::string toString() const override { return name + " instance"; }
    size_t arity() const override { return 0; }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) override;
    std::string getName() const { return name; }
    // Optionally, add a getter for methods if needed
    // const auto& getMethods() const { return methods; }
    std::shared_ptr<LoxFunction> findMethod(const std::string& name) const {
        auto it = methods.find(name);
        if (it != methods.end()) {
            return it->second;
        }
        return nullptr; // Method not found
    }
private:
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
};