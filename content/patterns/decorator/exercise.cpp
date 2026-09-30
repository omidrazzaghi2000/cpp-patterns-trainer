// Exercise: price a cloud server plan by stacking add-ons as Decorators.
#include <iostream>
#include <memory>
#include <string>
#include <utility>

// Component
class Plan {
public:
    virtual ~Plan() = default;
    virtual int monthlyPrice() const = 0;
    virtual std::string describe() const = 0;
};

// Concrete component
class BasicServer : public Plan {
public:
    int monthlyPrice() const override { return 10; }
    std::string describe() const override { return "2 vCPU server"; }
};

// Base decorator: is a Plan, wraps a Plan, forwards everything.
class AddOn : public Plan {
public:
    explicit AddOn(std::unique_ptr<Plan> inner) : inner_(std::move(inner)) {}
    int monthlyPrice() const override { return inner_->monthlyPrice(); }
    std::string describe() const override { return inner_->describe(); }

private:
    std::unique_ptr<Plan> inner_;
};

// A finished add-on, to use as a model.
class Backups : public AddOn {
public:
    using AddOn::AddOn;
    int monthlyPrice() const override { return AddOn::monthlyPrice() + 4; }
    std::string describe() const override { return AddOn::describe() + " + backups"; }
};

class Monitoring : public AddOn {
public:
    using AddOn::AddOn;
    // TODO 1: override monthlyPrice(): the wrapped plan's price + 3.
    // TODO 2: override describe(): the wrapped description + " + monitoring".
};

// TODO 3: write a `StaticIp` add-on: + 2 per month, appends " + static IP".

void print(const std::string& label, const Plan& plan) {
    std::cout << label << ": " << plan.describe() << " = $" << plan.monthlyPrice() << '\n';
}

int main() {
    auto dev = std::make_unique<BasicServer>();

    std::unique_ptr<Plan> prod = std::make_unique<Backups>(std::make_unique<BasicServer>());
    // TODO 4: wrap `prod` in Monitoring and then in StaticIp, e.g.
    //         prod = std::make_unique<Monitoring>(std::move(prod));

    print("dev", *dev);
    print("prod", *prod);
}
