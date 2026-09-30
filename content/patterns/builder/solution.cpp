// Solution: every step records its part and returns *this; build() validates the whole.
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class PizzaOrder {
public:
    class Builder;

    void print() const {
        std::cout << size_ << ", " << crust_ << " crust:";
        if (toppings_.empty()) std::cout << " plain";
        for (const auto& topping : toppings_) std::cout << ' ' << topping;
        if (extraCheese_) std::cout << " +extra cheese";
        std::cout << '\n';
    }

private:
    PizzaOrder() = default;  // only the Builder can create an order
    std::string size_ = "medium";
    std::string crust_ = "classic";
    std::vector<std::string> toppings_;
    bool extraCheese_ = false;
};

class PizzaOrder::Builder {
public:
    Builder& size(std::string s) { order_.size_ = std::move(s); return *this; }
    Builder& crust(std::string c) { order_.crust_ = std::move(c); return *this; }

    Builder& addTopping(std::string topping) {
        order_.toppings_.push_back(std::move(topping));
        return *this;
    }

    Builder& extraCheese() {
        order_.extraCheese_ = true;
        return *this;
    }

    PizzaOrder build() const {
        if (order_.toppings_.size() > 3)
            throw std::invalid_argument("at most 3 toppings");
        return order_;
    }

private:
    PizzaOrder order_;
};

// A director: a named recipe that runs a fixed sequence of builder steps.
PizzaOrder::Builder margherita() {
    PizzaOrder::Builder builder;
    builder.addTopping("tomato").addTopping("mozzarella").addTopping("basil");
    return builder;
}

int main() {
    PizzaOrder::Builder().size("large").crust("thin")
        .addTopping("mushrooms").addTopping("olives").extraCheese()
        .build().print();
    PizzaOrder::Builder().size("small").build().print();
    margherita().size("large").build().print();  // start from a recipe, then tweak it

    try {
        PizzaOrder::Builder().addTopping("ham").addTopping("corn")
            .addTopping("onion").addTopping("pepper").build().print();
    } catch (const std::invalid_argument& e) {
        std::cout << "rejected: " << e.what() << '\n';
    }
}
