// Exercise: finish the interpreter for a tiny warehouse-robot script language.
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

// Context: the robot's state, updated while the program is interpreted.
struct Robot {
    int x = 0, y = 0;
    int heading = 0;  // 0 = north, 1 = east, 2 = south, 3 = west
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void interpret(Robot& robot) const = 0;
};
using StmtPtr = std::unique_ptr<Stmt>;

// Terminal: "drive n" moves n cells in the current heading.
class Drive : public Stmt {
public:
    explicit Drive(int cells) : cells_(cells) {}
    void interpret(Robot& r) const override {
        constexpr int dx[] = {0, 1, 0, -1};
        constexpr int dy[] = {1, 0, -1, 0};
        r.x += dx[r.heading] * cells_;
        r.y += dy[r.heading] * cells_;
        std::cout << "  drive " << cells_ << " -> (" << r.x << ", " << r.y << ")\n";
    }

private:
    int cells_;
};

// Terminal: "right" turns 90 degrees clockwise.
class TurnRight : public Stmt {
public:
    void interpret(Robot& r) const override { r.heading = (r.heading + 1) % 4; }
};

// Nonterminal: "{ s1; s2; ... }" runs its statements in order.
class Block : public Stmt {
public:
    explicit Block(std::vector<StmtPtr> body) : body_(std::move(body)) {}
    void interpret(Robot& r) const override {
        // TODO 1: interpret EVERY statement of body_ in order, not only the first one.
        if (!body_.empty()) body_.front()->interpret(r);
    }

private:
    std::vector<StmtPtr> body_;
};

// Nonterminal: "repeat n { ... }" interprets the same subtree n times.
class Repeat : public Stmt {
public:
    Repeat(int times, StmtPtr body) : times_(times), body_(std::move(body)) {}
    void interpret(Robot& r) const override {
        // TODO 2: interpret body_ times_ times (it currently runs only once).
        body_->interpret(r);
    }

private:
    int times_;
    StmtPtr body_;
};

// Helpers that build the syntax tree (a parser would normally do this).
StmtPtr drive(int cells) { return std::make_unique<Drive>(cells); }
StmtPtr right() { return std::make_unique<TurnRight>(); }
StmtPtr repeat(int n, StmtPtr body) { return std::make_unique<Repeat>(n, std::move(body)); }
template <class... S>
StmtPtr block(S... stmts) {
    std::vector<StmtPtr> body;
    (body.push_back(std::move(stmts)), ...);  // unique_ptr can't go in an initializer_list
    return std::make_unique<Block>(std::move(body));
}

int main() {
    // Patrol one aisle and come back:  repeat 2 { drive 3; right; drive 2; right }
    const StmtPtr patrol = repeat(2, block(drive(3), right(), drive(2), right()));

    Robot robot;
    patrol->interpret(robot);
    std::cout << "final: (" << robot.x << ", " << robot.y << ") facing "
              << "NESW"[robot.heading] << '\n';
}
