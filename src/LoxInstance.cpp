#include "LoxInstance.hpp"
#include "LoxClass.hpp"

std::string LoxInstance::toString() const {
    return klass->getName() + " instance";
}
