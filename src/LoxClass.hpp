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
    LoxClass(const std::string& name, std::weak_ptr<LoxClass> superclass, const std::unordered_map<std::string, std::shared_ptr<LoxFunction>>& methods)
        : name(name), superclass(superclass), methods(methods) {}
    std::string toString() const override { return name; } // Only return class name
    size_t arity() const override {
        auto initializer = findMethod("init");
        if (initializer == nullptr) return 0;
        return initializer->arity();
    }
    lox_literal call(const Interpreter&, const std::vector<lox_literal>&) override;
    std::string getName() const { return name; }
    std::shared_ptr<LoxFunction> findMethod(const std::string& name) const {
        auto it = methods.find(name);
        if (it != methods.end()) {
            return it->second;
        }
        // If not found, check superclass if it exists
        if (auto super = superclass.lock()) {
            return super->findMethod(name);
        }
        return nullptr; // Method not found
    }
private:
    std::string name;
    std::weak_ptr<LoxClass> superclass; // Optional, can be empty if no superclass
    std::unordered_map<std::string, std::shared_ptr<LoxFunction>> methods;
};