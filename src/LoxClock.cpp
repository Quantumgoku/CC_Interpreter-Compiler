#include <ctime>
lox_literal LoxClock::call(const Interpreter&, const std::vector<lox_literal>&) const {
    return static_cast<double>(std::time(nullptr));
}