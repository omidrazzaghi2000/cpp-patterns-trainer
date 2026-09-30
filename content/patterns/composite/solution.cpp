// Solution: departments hold OrgUnits, so teams nest and costs add up recursively.
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Component: anything in the org chart that costs money every month.
class OrgUnit {
public:
    virtual ~OrgUnit() = default;
    virtual int cost() const = 0;
    virtual void print(int depth) const = 0;
};

// Leaf
class Employee : public OrgUnit {
public:
    Employee(std::string name, int salary) : name_(std::move(name)), salary_(salary) {}
    int cost() const override { return salary_; }
    void print(int depth) const override {
        std::cout << std::string(depth * 2, ' ') << name_ << ": " << salary_ << '\n';
    }

private:
    std::string name_;
    int salary_;
};

// Composite
class Department : public OrgUnit {
public:
    explicit Department(std::string name) : name_(std::move(name)) {}

    void add(std::unique_ptr<OrgUnit> member) { members_.push_back(std::move(member)); }

    int cost() const override {
        int total = 0;
        for (const auto& member : members_) total += member->cost();  // leaf or subtree
        return total;
    }

    void print(int depth) const override {
        std::cout << std::string(depth * 2, ' ') << name_ << " [" << cost() << "]\n";
        for (const auto& member : members_) member->print(depth + 1);
    }

private:
    std::string name_;
    std::vector<std::unique_ptr<OrgUnit>> members_;
};

int main() {
    auto backend = std::make_unique<Department>("Backend");
    backend->add(std::make_unique<Employee>("Sara", 5200));
    backend->add(std::make_unique<Employee>("Reza", 4800));

    Department engineering("Engineering");
    engineering.add(std::make_unique<Employee>("Neda (CTO)", 9000));
    engineering.add(std::move(backend));

    engineering.print(0);
    std::cout << "total payroll: " << engineering.cost() << '\n';
}
