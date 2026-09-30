// Exercise: add a new operation - shipping cost - to the shop's cart items with a Visitor.
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Book; struct Laptop; struct Grocery;

class ItemVisitor {
public:
    virtual ~ItemVisitor() = default;
    virtual void visit(const Book& b) = 0;
    virtual void visit(const Laptop& l) = 0;
    virtual void visit(const Grocery& g) = 0;
};

struct Item {
    virtual ~Item() = default;
    virtual void accept(ItemVisitor& v) const = 0;
};

struct Book final : Item {
    explicit Book(std::string t) : title(std::move(t)) {}
    void accept(ItemVisitor& v) const override { v.visit(*this); }
    std::string title;
};

struct Laptop final : Item {
    explicit Laptop(int kg) : weightKg(kg) {}
    void accept(ItemVisitor& v) const override { v.visit(*this); }
    int weightKg;
};

struct Grocery final : Item {
    Grocery(std::string n, bool c) : name(std::move(n)), chilled(c) {}
    // TODO 1: implement accept() like the other items do (right now it does nothing).
    void accept(ItemVisitor&) const override {}
    std::string name;
    bool chilled;
};

// An existing operation that is already written as a visitor.
class Describer final : public ItemVisitor {
public:
    void visit(const Book& b) override { std::cout << "book: " << b.title << '\n'; }
    void visit(const Laptop& l) override { std::cout << "laptop: " << l.weightKg << " kg\n"; }
    void visit(const Grocery& g) override {
        std::cout << "grocery: " << g.name << (g.chilled ? " (chilled)" : "") << '\n';
    }
};

// TODO 2: implement the three visit() overloads so they add to total_:
//         Book = 3, Laptop = 5 + 2 per kg, Grocery = 4 (+6 more if chilled).
class ShippingCost final : public ItemVisitor {
public:
    void visit(const Book&) override {}
    void visit(const Laptop&) override {}
    void visit(const Grocery&) override {}
    int total() const { return total_; }

private:
    int total_ = 0;
};

int main() {
    std::vector<std::unique_ptr<Item>> cart;
    cart.push_back(std::make_unique<Book>("Dune"));
    cart.push_back(std::make_unique<Laptop>(2));
    cart.push_back(std::make_unique<Grocery>("ice cream", true));
    cart.push_back(std::make_unique<Grocery>("rice", false));

    Describer describe;
    for (const auto& item : cart) item->accept(describe);

    ShippingCost shipping;
    // TODO 3: send the shipping visitor to every item in the cart.
    std::cout << "shipping: " << shipping.total() << " EUR\n";
}
