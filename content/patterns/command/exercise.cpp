// Exercise: make smart-thermostat changes undoable with the Command pattern.
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
        // TODO 1: remember the current target in previous_ BEFORE changing it.
        thermostat_.setTarget(celsius_);
    }

    void undo() override {
        // TODO 2: put the remembered target back on the thermostat.
    }

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
        // TODO 3: keep the executed command in history_ so it can be undone later.
    }

    void undoLast() {
        // TODO 4: if history_ is not empty, undo the newest command and drop it.
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
