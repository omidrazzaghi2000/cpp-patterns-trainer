// Interpreter: a tiny rule language for feature flags, evaluated for each user.
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Context: the facts a rule is interpreted against.
struct User {
    std::string name;
    std::set<std::string> traits;  // e.g. "beta", "premium", "trial"
};

// AbstractExpression: every grammar rule becomes a class with interpret().
class Expr {
public:
    virtual ~Expr() = default;
    virtual bool interpret(const User& user) const = 0;
    virtual std::string text() const = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

// TerminalExpression: a leaf of the grammar - "does the user have this trait?"
class Trait : public Expr {
public:
    explicit Trait(std::string name) : name_(std::move(name)) {}
    bool interpret(const User& u) const override { return u.traits.contains(name_); }
    std::string text() const override { return name_; }
private:
    std::string name_;
};

// NonterminalExpressions: hold sub-expressions and interpret them recursively.
class And : public Expr {
public:
    And(ExprPtr l, ExprPtr r) : l_(std::move(l)), r_(std::move(r)) {}
    bool interpret(const User& u) const override {
        return l_->interpret(u) && r_->interpret(u);  // recurse into both children
    }
    std::string text() const override { return "(" + l_->text() + " AND " + r_->text() + ")"; }
private:
    ExprPtr l_, r_;
};

class Or : public Expr {
public:
    Or(ExprPtr l, ExprPtr r) : l_(std::move(l)), r_(std::move(r)) {}
    bool interpret(const User& u) const override {
        return l_->interpret(u) || r_->interpret(u);
    }
    std::string text() const override { return "(" + l_->text() + " OR " + r_->text() + ")"; }
private:
    ExprPtr l_, r_;
};

class Not : public Expr {
public:
    explicit Not(ExprPtr e) : e_(std::move(e)) {}
    bool interpret(const User& u) const override { return !e_->interpret(u); }
    std::string text() const override { return "NOT " + e_->text(); }
private:
    ExprPtr e_;
};

// Small helpers, so building the syntax tree reads almost like the rule itself.
ExprPtr is(std::string trait) { return std::make_unique<Trait>(std::move(trait)); }
ExprPtr negate(ExprPtr e) { return std::make_unique<Not>(std::move(e)); }
ExprPtr both(ExprPtr a, ExprPtr b) {
    return std::make_unique<And>(std::move(a), std::move(b));
}
ExprPtr either(ExprPtr a, ExprPtr b) {
    return std::make_unique<Or>(std::move(a), std::move(b));
}

int main() {
    // A parser would normally build this tree from the rule's source text:
    //   new_checkout := beta AND (premium OR NOT trial)
    const ExprPtr rule = both(is("beta"), either(is("premium"), negate(is("trial"))));
    std::cout << "rule: " << rule->text() << '\n';

    const std::vector<User> users{{"sara", {"beta", "premium"}},
                                  {"reza", {"beta", "trial"}},
                                  {"mina", {"premium"}},
                                  {"kian", {"beta"}}};
    for (const User& u : users) {
        std::cout << u.name << ": " << (rule->interpret(u) ? "ON" : "off") << '\n';
    }
}
