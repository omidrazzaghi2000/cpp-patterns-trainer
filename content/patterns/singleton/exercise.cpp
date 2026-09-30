// Exercise: turn IdGenerator into a Singleton so every module shares ONE
// counter and IDs are never repeated.
#include <iostream>
#include <string>

class IdGenerator {
public:
    // TODO 1: make this constructor private.
    // TODO 2: add   static IdGenerator& instance()   that returns the one object
    //         (hint: a function-local static variable).
    // TODO 3: delete the copy constructor and the copy assignment operator.
    IdGenerator() = default;

    int next() { return ++last_; }

private:
    int last_ = 0;
};

std::string createUser() {
    IdGenerator gen;  // TODO 4: use IdGenerator::instance() instead
    return "user#" + std::to_string(gen.next());
}

std::string createOrder() {
    IdGenerator gen;  // TODO 4: use IdGenerator::instance() instead
    return "order#" + std::to_string(gen.next());
}

int main() {
    std::cout << createUser() << '\n';
    std::cout << createOrder() << '\n';
    std::cout << createUser() << '\n';
}
