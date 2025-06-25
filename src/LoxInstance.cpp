#include "LoxInstance.hpp"
#include "LoxClass.hpp"

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> klass) : klass(std::move(klass)) {}

LoxInstance::~LoxInstance() {}

std::string LoxInstance::toString() const {
    return klass->getName() + " instance";
}

lox_literal LoxInstance::call(Interpreter& interpreter, const std::vector<lox_literal>&) {
    throw RuntimeError(Token(TokenType::IDENTIFIER, "", std::monostate{}, 0), "Only classes and functions are callable.");
}

lox_literal LoxInstance::get(const Token& name) {
    // Defensive: check klass is valid
    if (!klass) {
        throw RuntimeError(name, "Internal error: instance has no class.");
    }
    try {
        auto it = fields.find(name.getLexeme());
        if (it != fields.end()) {
            // Per Crafting Interpreters: if the property is in fields, return as-is (do NOT rebind)
            return it->second;
        }
        std::shared_ptr<LoxFunction> method = klass->findMethod(name.getLexeme());
        if (method) {
            // Per Crafting Interpreters: if found on class, bind to this instance
            try {
                return method->bind(shared_from_this());
            } catch (const std::bad_weak_ptr&) {
                throw RuntimeError(name, "Internal error: invalid instance reference.");
            }
        }
        throw RuntimeError(name, "Undefined property '" + name.getLexeme() + "'.");
    } catch (const RuntimeError&) {
        throw; // Already a RuntimeError, propagate
    } catch (const std::exception& e) {
        throw RuntimeError(name, std::string("Internal error: ") + e.what());
    } catch (...) {
        throw RuntimeError(name, "Internal error: unknown exception in property access.");
    }
}

void LoxInstance::set(const Token& name, const lox_literal& value) {
    // If assigning a function, always rebind the original (unbound) function to this instance
    if (std::holds_alternative<std::shared_ptr<LoxCallable>>(value)) {
        auto func = std::dynamic_pointer_cast<LoxFunction>(std::get<std::shared_ptr<LoxCallable>>(value));
        if (func) {
            // Defensive: always unwrap to the original function before rebinding
            auto orig = func->getUnbound();
            fields[name.getLexeme()] = orig->bind(shared_from_this());
            return;
        }
    }
    // For all other values, just store as-is
    fields[name.getLexeme()] = value;
}
