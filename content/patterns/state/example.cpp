// State: a vending machine whose coin slot and button behave differently in each state.
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>

class VendingMachine;
class MachineState;
using StatePtr = std::unique_ptr<MachineState>;

// State interface: one handler per event. A handler returns the next state,
// or nullptr to stay put, so every transition is explicit and easy to trace.
class MachineState {
public:
    virtual ~MachineState() = default;
    virtual std::string_view name() const = 0;
    virtual StatePtr onCoin(VendingMachine& machine) const = 0;
    virtual StatePtr onButton(VendingMachine& machine) const = 0;
protected:
    static StatePtr stay(std::string_view why) {
        std::cout << "  " << why << '\n';
        return nullptr;
    }
};

// Context: holds the current state and the shared data (the stock).
class VendingMachine {
public:
    explicit VendingMachine(int snacks);
    void insertCoin()  { announce("insert coin");  transitionTo(state_->onCoin(*this)); }
    void pressButton() { announce("press button"); transitionTo(state_->onButton(*this)); }
    int dispense() { return --stock_; }
private:
    void announce(std::string_view event) const {
        std::cout << '[' << state_->name() << "] " << event << '\n';
    }
    void transitionTo(StatePtr next) {
        if (!next) return;  // the state decided to stay
        std::cout << "  " << state_->name() << " -> " << next->name() << '\n';
        state_ = std::move(next);
    }
    int stock_;
    StatePtr state_;
};

// Concrete states: each one knows its own behaviour and its successors.
class SoldOut : public MachineState {
public:
    std::string_view name() const override { return "SoldOut"; }
    StatePtr onCoin(VendingMachine&) const override { return stay("sold out, coin returned"); }
    StatePtr onButton(VendingMachine&) const override { return stay("sold out"); }
};

class Idle : public MachineState {
public:
    std::string_view name() const override { return "Idle"; }
    StatePtr onCoin(VendingMachine&) const override;  // defined below: needs HasCoin
    StatePtr onButton(VendingMachine&) const override { return stay("insert a coin first"); }
};

class HasCoin : public MachineState {
public:
    std::string_view name() const override { return "HasCoin"; }
    StatePtr onCoin(VendingMachine&) const override { return stay("extra coin returned"); }
    StatePtr onButton(VendingMachine& machine) const override {
        const int left = machine.dispense();
        std::cout << "  snack dispensed, " << left << " left\n";
        if (left == 0) return std::make_unique<SoldOut>();
        return std::make_unique<Idle>();
    }
};

StatePtr Idle::onCoin(VendingMachine&) const {
    std::cout << "  coin accepted\n";
    return std::make_unique<HasCoin>();
}

VendingMachine::VendingMachine(int snacks)
    : stock_(snacks), state_(std::make_unique<Idle>()) {}

int main() {
    VendingMachine machine(2);
    machine.pressButton();  // Idle: nothing to pay for yet
    machine.insertCoin();   // Idle -> HasCoin
    machine.insertCoin();   // HasCoin: rejects a second coin
    machine.pressButton();  // HasCoin -> Idle (one snack left)
    machine.insertCoin();   // Idle -> HasCoin
    machine.pressButton();  // HasCoin -> SoldOut (last snack)
    machine.insertCoin();   // SoldOut: gives the coin back
}
