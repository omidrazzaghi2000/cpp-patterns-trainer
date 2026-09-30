// Solution: smart-thermostat changes as commands that remember how to undo themselves.
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

// Receiver
class Thermostat {
public:
    int target() const { return target_; }
    void setTarget(int celsius) { target_ = celsius; }

private:
    int target_ = 20;
};

// Command interface
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class SetTemperature : public Command {
public:
    SetTemperature(Thermostat& t, int celsius) : thermostat_(t), celsius_(celsius) {}

    void execute() override {
        previous_ = thermostat_.target();  // snapshot first, then change
        thermostat_.setTarget(celsius_);
    }

    void undo() override { thermostat_.setTarget(previous_); }

private:
    Thermostat& thermostat_;
    int celsius_;
    int previous_ = 0;  // the state undo() needs
};

// Invoker: the phone app's buttons
class RemoteApp {
public:
    void press(std::unique_ptr<Command> cmd) {
        cmd->execute();
        history_.push_back(std::move(cmd));
    }

    void undoLast() {
        if (history_.empty()) return;
        history_.back()->undo();
        history_.pop_back();
    }

private:
    std::vector<std::unique_ptr<Command>> history_;
};

int main() {
    Thermostat living;
    RemoteApp app;
    std::cout << "start: " << living.target() << "C\n";

    app.press(std::make_unique<SetTemperature>(living, 23));
    app.press(std::make_unique<SetTemperature>(living, 18));
    std::cout << "after two changes: " << living.target() << "C\n";

    app.undoLast();
    std::cout << "undo: " << living.target() << "C\n";
    app.undoLast();
    std::cout << "undo: " << living.target() << "C\n";
    app.undoLast();  // history is empty now: must do nothing
    std::cout << "undo again: " << living.target() << "C\n";
}
