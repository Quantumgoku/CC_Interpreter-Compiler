#include "LoxInstance.hpp"
#include "LoxClass.hpp"

std::string LoxInstance::toString() const {
    return klass->getName() + " instance";
}

void LoxInstance::set(const Token& name, const lox_literal& value) {
    fields[name.getLexeme()] = value;
}
