// Solution: NullNotifier stands in for "no channel", so main() needs no checks.
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Notifier {
public:
    virtual ~Notifier() = default;
    virtual void send(std::string_view customer, std::string_view text) const = 0;
    virtual std::string_view channel() const = 0;
};

class SmsNotifier final : public Notifier {
public:
    void send(std::string_view customer, std::string_view text) const override {
        std::cout << "  SMS to " << customer << ": " << text << '\n';
    }
    std::string_view channel() const override { return "sms"; }
};

class PushNotifier final : public Notifier {
public:
    void send(std::string_view customer, std::string_view text) const override {
        std::cout << "  push to " << customer << ": " << text << '\n';
    }
    std::string_view channel() const override { return "push"; }
};

class NullNotifier final : public Notifier {
public:
    void send(std::string_view, std::string_view) const override {}  // do nothing
    std::string_view channel() const override { return "none"; }
};

std::unique_ptr<Notifier> makeNotifier(std::string_view preference) {
    if (preference == "sms") return std::make_unique<SmsNotifier>();
    if (preference == "push") return std::make_unique<PushNotifier>();
    return std::make_unique<NullNotifier>();  // opted out: silently ignore
}

struct Customer {
    std::string name;
    std::unique_ptr<Notifier> notifier;  // must never be null
};

int main() {
    std::vector<Customer> customers;
    customers.push_back({"Sara", makeNotifier("push")});
    customers.push_back({"Omid", makeNotifier("none")});
    customers.push_back({"Lena", makeNotifier("sms")});

    for (const Customer& c : customers) {
        std::cout << c.name << " (" << c.notifier->channel() << ")\n";
        c.notifier->send(c.name, "your pizza is on the way");  // no null check
    }
}
