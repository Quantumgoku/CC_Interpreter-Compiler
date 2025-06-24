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
            // If the value is a function, always rebind it to this instance
            if (std::holds_alternative<std::shared_ptr<LoxCallable>>(it->second)) {
                auto func = std::dynamic_pointer_cast<LoxFunction>(std::get<std::shared_ptr<LoxCallable>>(it->second));
                if (func) {
                    return func->bind(shared_from_this());
                }
            }
            return it->second;
        }
        std::shared_ptr<LoxFunction> method = klass->findMethod(name.getLexeme());
        if (method) {
            // Defensive: check shared_from_this is valid
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
    // If assigning a function, always rebind it to this instance
    if (std::holds_alternative<std::shared_ptr<LoxCallable>>(value)) {
        auto func = std::dynamic_pointer_cast<LoxFunction>(std::get<std::shared_ptr<LoxCallable>>(value));
        if (func) {
            fields[name.getLexeme()] = func->bind(shared_from_this());
            return;
        }
    }
    fields[name.getLexeme()] = value;
}
