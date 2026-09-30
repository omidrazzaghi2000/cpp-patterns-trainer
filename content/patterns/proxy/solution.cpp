// Solution: a protection proxy that checks roles before touching the payroll database.
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>

enum class Role { Intern, Manager, HrAdmin };  // ordered from least to most rights

// Subject
class Payroll {
public:
    virtual ~Payroll() = default;
    virtual std::optional<int> salaryOf(const std::string& name) const = 0;
    virtual void giveRaise(const std::string& name, int amount) = 0;
};

// RealSubject: no security at all - it trusts every caller.
class PayrollDatabase : public Payroll {
public:
    std::optional<int> salaryOf(const std::string& name) const override {
        auto it = salaries_.find(name);
        return it != salaries_.end() ? std::optional<int>(it->second) : std::nullopt;
    }
    void giveRaise(const std::string& name, int amount) override {
        salaries_[name] += amount;
    }

private:
    std::map<std::string, int> salaries_{{"dana", 5200}, {"omid", 4800}};
};

// Proxy: same interface, checks the caller's role before delegating.
class SecurePayroll : public Payroll {
public:
    SecurePayroll(std::shared_ptr<Payroll> real, Role role)
        : real_(std::move(real)), role_(role) {}

    std::optional<int> salaryOf(const std::string& name) const override {
        if (role_ < Role::Manager) {  // interns may not read salaries
            std::cout << "  denied: salary of " << name << '\n';
            return std::nullopt;
        }
        return real_->salaryOf(name);
    }

    void giveRaise(const std::string& name, int amount) override {
        if (role_ != Role::HrAdmin) {  // the request never reaches the database
            std::cout << "  denied: raise for " << name << '\n';
            return;
        }
        real_->giveRaise(name, amount);
    }

private:
    std::shared_ptr<Payroll> real_;  // shared: several sessions use one database
    Role role_;
};

// Client code only knows the Payroll interface.
void session(const std::string& who, Payroll& payroll) {
    std::cout << who << ":\n";
    payroll.giveRaise("omid", 300);
    if (auto salary = payroll.salaryOf("omid")) {
        std::cout << "  omid earns " << *salary << '\n';
    }
}

int main() {
    auto db = std::make_shared<PayrollDatabase>();
    SecurePayroll intern(db, Role::Intern);
    SecurePayroll lead(db, Role::Manager);
    SecurePayroll hr(db, Role::HrAdmin);

    session("intern", intern);
    session("lead", lead);
    session("hr", hr);
}
