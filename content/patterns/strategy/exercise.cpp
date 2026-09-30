// Exercise: let the box office swap its ticket-pricing strategy at runtime.
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

// Formats cents as dollars, e.g. 960 -> "$9.60".
std::string dollars(int cents) {
    return "$" + std::to_string(cents / 100) + "." + std::to_string(cents % 100 / 10) +
           std::to_string(cents % 10);
}

// Strategy interface: how much does one ticket cost?
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual std::string_view label() const = 0;
    virtual int priceCents(int baseCents) const = 0;
};

class RegularPricing : public PricingStrategy {
public:
    std::string_view label() const override { return "regular"; }
    int priceCents(int baseCents) const override { return baseCents; }
};

class HappyHourPricing : public PricingStrategy {
public:
    std::string_view label() const override { return "happy hour"; }
    int priceCents(int baseCents) const override { return baseCents / 2; }
};

// TODO 1: add a StudentPricing strategy: label "student", 20% off the base price
//         (return baseCents * 80 / 100).

// Context: sells tickets and delegates the pricing rule to its strategy.
class BoxOffice {
public:
    explicit BoxOffice(std::unique_ptr<PricingStrategy> pricing)
        : pricing_(std::move(pricing)) {}

    // TODO 2: add setPricing(std::unique_ptr<PricingStrategy>) that replaces the
    //         current strategy (move the new one into pricing_).

    void sell(std::string_view movie, int tickets) const {
        const int total = tickets * pricing_->priceCents(kBaseCents);
        std::cout << tickets << " x " << movie << " [" << pricing_->label() << "]: "
                  << dollars(total) << '\n';
    }

private:
    static constexpr int kBaseCents = 1200;  // $12.00 per ticket
    std::unique_ptr<PricingStrategy> pricing_;
};

int main() {
    BoxOffice office(std::make_unique<RegularPricing>());
    office.sell("Dune", 2);

    // TODO 3: 5 p.m., happy hour starts: switch to HappyHourPricing before this sale.
    office.sell("Up", 3);

    // TODO 3: a student shows their card: switch to StudentPricing before this sale.
    office.sell("Heat", 1);
}
