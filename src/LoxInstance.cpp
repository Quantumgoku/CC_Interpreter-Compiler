#include "LoxInstance.hpp"
#include "LoxClass.hpp"

LoxInstance::LoxInstance(std::shared_ptr<LoxClass> klass) : klass(std::move(klass)) {}

std::string LoxInstance::toString() const {
    return klass->getName() + " instance";
}

lox_literal LoxInstance::call(const Interpreter&, const std::vector<lox_literal>&) {
    throw RuntimeError(Token(TokenType::IDENTIFIER, "", std::monostate{}, 0), "Only classes and functions are callable.");
}

lox_literal LoxInstance::get(const Token& name) {
    auto it = fields.find(name.getLexeme());
    if (it != fields.end()) {
        return it->second;
    }
    std::shared_ptr<LoxFunction> method = klass->findMethod(name.getLexeme());
    if(method != nullptr) {
        return method;
    }
    throw RuntimeError(name, "Undefined property '" + name.getLexeme() + "'.");
}

void LoxInstance::set(const Token& name, const lox_literal& value) {
    fields[name.getLexeme()] = value;
}
