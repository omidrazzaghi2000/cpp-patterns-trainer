// Solution: the box office delegates ticket pricing to an interchangeable strategy.
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

class StudentPricing : public PricingStrategy {
public:
    std::string_view label() const override { return "student"; }
    int priceCents(int baseCents) const override { return baseCents * 80 / 100; }
};

// Context: sells tickets and delegates the pricing rule to its strategy.
class BoxOffice {
public:
    explicit BoxOffice(std::unique_ptr<PricingStrategy> pricing)
        : pricing_(std::move(pricing)) {}

    void setPricing(std::unique_ptr<PricingStrategy> pricing) {
        pricing_ = std::move(pricing);
    }

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

    office.setPricing(std::make_unique<HappyHourPricing>());  // 5 p.m.: happy hour starts
    office.sell("Up", 3);

    office.setPricing(std::make_unique<StudentPricing>());  // a student shows their card
    office.sell("Heat", 1);
}
