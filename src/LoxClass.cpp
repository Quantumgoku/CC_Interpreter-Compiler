#include "LoxClass.hpp"
#include "LoxInstance.hpp"

lox_literal LoxClass::call(const Interpreter&, const std::vector<lox_literal>&) {
    auto instance = std::make_shared<LoxInstance>(std::static_pointer_cast<LoxClass>(shared_from_this()));
    return instance;
}
