// Solution: Traced<Derived> reaches the derived tag() through a compile-time cast.
#include <iostream>
#include <string>
#include <string_view>

// Mixin: adds trace() to any service. It needs the service's tag(), which it can
// reach at compile time by casting itself to the derived type - no virtual needed.
template <typename Derived>
class Traced {
public:
    void trace(std::string_view message) const {
        std::cout << '[' << derived().tag() << "] " << message << '\n';
    }

private:
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

class PaymentService : public Traced<PaymentService> {
public:
    std::string_view tag() const { return "payments"; }
    void charge(int cents) const { trace("charging " + std::to_string(cents) + " cents"); }
    void refund(int cents) const { trace("refunding " + std::to_string(cents) + " cents"); }
};

class EmailService : public Traced<EmailService> {
public:
    std::string_view tag() const { return "email"; }
    void send(std::string_view to) const { trace("sending receipt to " + std::string(to)); }
};

int main() {
    const PaymentService payments;
    const EmailService email;
    payments.charge(1999);
    email.send("ana@example.com");
    payments.refund(500);
}
