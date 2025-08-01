#include "LoxClass.hpp"
#include "LoxInstance.hpp"

lox_literal LoxClass::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    auto instance = LoxInstance::create(std::static_pointer_cast<LoxClass>(shared_from_this()));
    auto initializer = findMethod("init");
    if (initializer) {
        auto initFunc = std::dynamic_pointer_cast<LoxFunction>(initializer);
        if (initFunc) {
            initFunc->bind(instance)->call(interpreter, arguments);
        }
    }
    return instance;
}
