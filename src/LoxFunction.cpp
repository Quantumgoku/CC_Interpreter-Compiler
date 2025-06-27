#include "LoxFunction.hpp"
#include "interpreter.hpp"
#include <iostream>

lox_literal LoxFunction::call(Interpreter& interpreter, const std::vector<lox_literal>& arguments) {
    // Always create a new environment for the function, with closure as parent
    auto environment = std::make_shared<Environment>(closure);

    // If this is a method (closure has 'this'), propagate 'this' to the new environment
    if (closure->has("this")) {
        environment->define("this", closure->getAt(0, "this"));
    }
    
    // Store the arguments for the Block visitor to handle
    interpreter.pendingArguments = arguments;
    interpreter.pendingParams = declaration->params;
    
    auto prevClosure = interpreter.closureForNestedFunctions;
    interpreter.closureForNestedFunctions = environment;  // Set to current function's environment, not closure
    
    // Save the current interpreter environment
    auto prevEnv = interpreter.getCurrentEnvironment();
    interpreter.setCurrentEnvironment(environment);
    
    try {
        // Execute the function body statement (usually a Block) through the visitor
        interpreter.execute(*declaration->body);
    } catch (const ReturnException& returnValue) {
        interpreter.setCurrentEnvironment(prevEnv);
        interpreter.closureForNestedFunctions = prevClosure;
        if (isInitializer) {
            // Always return 'this' from the environment
            return environment->getAt(0, "this");
        }
        return returnValue.getValue();
    }
    interpreter.setCurrentEnvironment(prevEnv);
    interpreter.closureForNestedFunctions = prevClosure;
    if(isInitializer) {
        return environment->getAt(0, "this");
    }
    return lox_literal(std::monostate{});
}

std::shared_ptr<LoxFunction> LoxFunction::bind(const std::shared_ptr<LoxInstance>& instance) const {
    auto environment = std::make_shared<Environment>(closure);
    environment->define("this", instance);
    auto orig = getUnbound();
    return std::make_shared<LoxFunction>(orig->declaration, environment, orig->isInitializer, orig);
}

std::shared_ptr<LoxFunction> LoxFunction::getUnbound() const {
    if (original) {
        return original->getUnbound();
    }
    return std::const_pointer_cast<LoxFunction>(shared_from_this());
}
