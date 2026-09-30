// Solution: add-ons are Decorators, so any stack of them prices itself.
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

class Backups : public AddOn {
public:
    using AddOn::AddOn;
    int monthlyPrice() const override { return AddOn::monthlyPrice() + 4; }
    std::string describe() const override { return AddOn::describe() + " + backups"; }
};

class Monitoring : public AddOn {
public:
    using AddOn::AddOn;
    int monthlyPrice() const override { return AddOn::monthlyPrice() + 3; }
    std::string describe() const override { return AddOn::describe() + " + monitoring"; }
};

class StaticIp : public AddOn {
public:
    using AddOn::AddOn;
    int monthlyPrice() const override { return AddOn::monthlyPrice() + 2; }
    std::string describe() const override { return AddOn::describe() + " + static IP"; }
};

void print(const std::string& label, const Plan& plan) {
    std::cout << label << ": " << plan.describe() << " = $" << plan.monthlyPrice() << '\n';
}

int main() {
    auto dev = std::make_unique<BasicServer>();

    std::unique_ptr<Plan> prod = std::make_unique<Backups>(std::make_unique<BasicServer>());
    prod = std::make_unique<Monitoring>(std::move(prod));
    prod = std::make_unique<StaticIp>(std::move(prod));

    print("dev", *dev);
    print("prod", *prod);
}
