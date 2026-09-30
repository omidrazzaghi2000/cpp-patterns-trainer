// Facade: one placeOrder() call hides inventory, payment, shipping and email.
#include <iostream>
#include <string>
#include <string_view>

// ---- Subsystem: useful classes, but the client would have to know the right order ----
class Inventory {
public:
    bool reserve(std::string_view sku, int qty) {
        std::cout << "  inventory: reserved " << qty << " x " << sku << '\n';
        return true;  // a real one would return false when out of stock
    }
    void release(std::string_view sku, int qty) {
        std::cout << "  inventory: released " << qty << " x " << sku << '\n';
    }
};

class PaymentGateway {
public:
    bool charge(std::string_view card, int amount) {
        if (card.ends_with("0000")) {  // simulated decline
            std::cout << "  payment: card declined\n";
            return false;
        }
        std::cout << "  payment: charged $" << amount << '\n';
        return true;
    }
};

class Shipping {
public:
    std::string schedule(std::string_view city) {
        std::cout << "  shipping: courier booked to " << city << '\n';
        return "TRK-" + std::to_string(++lastTracking_);
    }

private:
    int lastTracking_ = 1000;
};

class Mailer {
public:
    void send(std::string_view to, std::string_view text) {
        std::cout << "  email to " << to << ": " << text << '\n';
    }
};

struct Order {
    std::string sku, card, email, city;
    int qty, amount;
};

// ---- Facade: the one simple entry point that coordinates the subsystem ----
class CheckoutService {
public:
    bool placeOrder(const Order& o) {
        if (!inventory_.reserve(o.sku, o.qty)) return false;
        if (!payments_.charge(o.card, o.amount)) {
            inventory_.release(o.sku, o.qty);  // rollback the client never has to think about
            return false;
        }
        const std::string tracking = shipping_.schedule(o.city);
        mailer_.send(o.email, "order shipped, tracking " + tracking);
        return true;
    }

private:
    Inventory inventory_;
    PaymentGateway payments_;
    Shipping shipping_;
    Mailer mailer_;
};

int main() {
    CheckoutService checkout;  // the client knows ONE class, not four
    const Order orders[] = {
        {"KEYBOARD-7", "4111-1111", "sara@mail.com", "Tabriz", 1, 49},
        {"MOUSE-3", "5500-0000", "reza@mail.com", "Shiraz", 2, 30},
    };
    for (int n = 1; const Order& order : orders) {
        std::cout << "order " << n++ << ":\n";
        const bool ok = checkout.placeOrder(order);
        std::cout << "  -> " << (ok ? "success" : "failed") << '\n';
    }
}
