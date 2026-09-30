// Exercise: teach the type-erased CartItem a price() operation so the checkout
// can total up product types that share no base class.
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Unrelated product types: none of them inherits from anything.
struct Book {
    std::string title;
    int cents;
    std::string name() const { return "Book \"" + title + "\""; }
    int price() const { return cents; }
};
struct EBook {
    std::string title;
    std::string name() const { return "E-book \"" + title + "\""; }
    int price() const { return 499; }
};
struct GiftCard {
    int cents;
    std::string name() const { return "Gift card"; }
    int price() const { return cents; }
};
struct Stickers {  // brand-new product line, added after CartItem was written
    int count;
    std::string name() const { return std::to_string(count) + " stickers"; }
    int price() const { return count * 150; }
};

class CartItem {
public:
    template <typename T>
    CartItem(T item) : self_(std::make_unique<Model<T>>(std::move(item))) {}

    CartItem(const CartItem& other) : self_(other.self_->clone()) {}
    CartItem& operator=(const CartItem& other) { return *this = CartItem(other); }
    CartItem(CartItem&&) noexcept = default;
    CartItem& operator=(CartItem&&) noexcept = default;

    std::string name() const { return self_->name(); }
    int price() const { return 0; }  // TODO 3: forward the call to the hidden object

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual std::string name() const = 0;
        // TODO 1: declare a pure virtual   int price() const
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    template <typename T>
    struct Model final : Concept {
        explicit Model(T i) : item(std::move(i)) {}
        std::string name() const override { return item.name(); }
        // TODO 2: override price() by delegating to the wrapped item
        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model>(*this);
        }
        T item;
    };

    std::unique_ptr<Concept> self_;
};

std::string money(int cents) {
    const std::string rest = std::to_string(cents % 100);
    return "$" + std::to_string(cents / 100) + "." + (rest.size() == 1 ? "0" : "") + rest;
}

int main() {
    std::vector<CartItem> cart{Book{"Dune", 1299}, EBook{"Neuromancer"}, GiftCard{2500}};
    // TODO 4: add Stickers{3} to the cart -- without touching CartItem.

    int total = 0;
    for (const CartItem& item : cart) {
        std::cout << item.name() << ": " << money(item.price()) << '\n';
        total += item.price();
    }
    std::cout << "total: " << money(total) << '\n';
}
