// Solution: an article moves Draft -> Review -> Published through explicit state objects.
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>

class ArticleState;
using StatePtr = std::unique_ptr<ArticleState>;

// State interface: each handler returns the next state, or nullptr to stay.
class ArticleState {
public:
    virtual ~ArticleState() = default;
    virtual std::string_view name() const = 0;
    virtual StatePtr submit() const = 0;
    virtual StatePtr approve() const = 0;
    virtual StatePtr reject() const = 0;
protected:
    static StatePtr stay(std::string_view why) {
        std::cout << "  " << why << '\n';
        return nullptr;
    }
};

class Published : public ArticleState {
public:
    std::string_view name() const override { return "Published"; }
    StatePtr submit() const override { return stay("already live"); }
    StatePtr approve() const override { return stay("already live"); }
    StatePtr reject() const override { return stay("too late, it is live"); }
};

class Draft : public ArticleState {
public:
    std::string_view name() const override { return "Draft"; }
    StatePtr submit() const override;  // defined below: needs Review
    StatePtr approve() const override { return stay("drafts cannot be approved"); }
    StatePtr reject() const override { return stay("nothing to reject"); }
};

class Review : public ArticleState {
public:
    std::string_view name() const override { return "Review"; }
    StatePtr submit() const override { return stay("already waiting for review"); }
    StatePtr approve() const override {
        std::cout << "  approved by the editor\n";
        return std::make_unique<Published>();
    }
    StatePtr reject() const override {
        std::cout << "  sent back for changes\n";
        return std::make_unique<Draft>();
    }
};

StatePtr Draft::submit() const {
    std::cout << "  sent to the editor\n";
    return std::make_unique<Review>();
}

// Context: forwards every action to the current state and applies the transition.
class Article {
public:
    void submit()  { announce("submit");  transitionTo(state_->submit()); }
    void approve() { announce("approve"); transitionTo(state_->approve()); }
    void reject()  { announce("reject");  transitionTo(state_->reject()); }

private:
    void announce(std::string_view action) const {
        std::cout << '[' << state_->name() << "] " << action << '\n';
    }
    void transitionTo(StatePtr next) {
        if (!next) return;
        std::cout << "  " << state_->name() << " -> " << next->name() << '\n';
        state_ = std::move(next);
    }

    StatePtr state_ = std::make_unique<Draft>();
};

int main() {
    Article post;
    post.approve();  // Draft: nothing to approve yet
    post.submit();   // Draft -> Review
    post.reject();   // Review -> Draft
    post.submit();   // Draft -> Review
    post.approve();  // Review -> Published
    post.reject();   // Published: too late
}
