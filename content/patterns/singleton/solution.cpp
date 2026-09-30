// Solution: IdGenerator as a Meyers' Singleton.
#include <iostream>
#include <string>

class IdGenerator {
public:
    static IdGenerator& instance() {
        static IdGenerator generator;  // created once, on first use
        return generator;
    }

    IdGenerator(const IdGenerator&) = delete;
    IdGenerator& operator=(const IdGenerator&) = delete;

    int next() { return ++last_; }

private:
    IdGenerator() = default;
    int last_ = 0;
};

std::string createUser() {
    return "user#" + std::to_string(IdGenerator::instance().next());
}

std::string createOrder() {
    return "order#" + std::to_string(IdGenerator::instance().next());
}

int main() {
    std::cout << createUser() << '\n';
    std::cout << createOrder() << '\n';
    std::cout << createUser() << '\n';
}
