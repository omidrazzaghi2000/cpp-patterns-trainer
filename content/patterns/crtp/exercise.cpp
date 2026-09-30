// Exercise: finish the CRTP mixin Traced<Derived> so every service logs with its own tag.
#include <iostream>
#include <string>
#include <string_view>

// Mixin: adds trace() to any service. It needs the service's tag(), which it can
// reach at compile time by casting itself to the derived type - no virtual needed.
template <typename Derived>
class Traced {
public:
    void trace(std::string_view message) const {
        // TODO 2: print "[<tag>] <message>", taking the tag from derived().tag().
        std::cout << "[?] " << message << '\n';
    }

private:
    // TODO 1: add a helper   const Derived& derived() const   that returns
    //         *this converted with static_cast<const Derived&>.
};

class PaymentService : public Traced<PaymentService> {
public:
    std::string_view tag() const { return "payments"; }
    void charge(int cents) const { trace("charging " + std::to_string(cents) + " cents"); }
    void refund(int cents) const { trace("refunding " + std::to_string(cents) + " cents"); }
};

// TODO 3: EmailService should get trace() as well: inherit from Traced<EmailService>
//         and let send() call trace("sending receipt to " + ...) instead of printing.
class EmailService {
public:
    std::string_view tag() const { return "email"; }
    void send(std::string_view to) const { std::cout << "(untraced) mail to " << to << '\n'; }
};

int main() {
    const PaymentService payments;
    const EmailService email;
    payments.charge(1999);
    email.send("ana@example.com");
    payments.refund(500);
}
