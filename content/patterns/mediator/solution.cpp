// Solution: the SmartHub mediator owns every rule about how home devices react to each other.
#include <iostream>
#include <string_view>

class Device;

// Mediator interface: devices report events here instead of calling each other.
class Hub {
public:
    virtual ~Hub() = default;
    virtual void notify(const Device& sender, std::string_view event) = 0;
};

class Device {
public:
    Device(Hub& hub, std::string_view name) : hub_(hub), name_(name) {}
    Device(const Device&) = delete;  // a device is wired to exactly one hub
    virtual ~Device() = default;
    std::string_view name() const { return name_; }
protected:
    void report(std::string_view event) const { hub_.notify(*this, event); }
private:
    Hub& hub_;
    std::string_view name_;
};

class DoorSensor : public Device {
public:
    using Device::Device;
    void open() const {
        std::cout << "door: opened\n";
        report("opened");
    }
};

class NightButton : public Device {
public:
    using Device::Device;
    void press() const {
        std::cout << "night button: pressed\n";
        report("pressed");
    }
};

class Lights : public Device {
public:
    using Device::Device;
    void turnOn() const { std::cout << "  lights: on\n"; }
    void turnOff() const { std::cout << "  lights: off\n"; }
};

class Alarm : public Device {
public:
    using Device::Device;
    void arm() {
        armed_ = true;
        std::cout << "  alarm: armed\n";
    }
    void ring() const { std::cout << "  alarm: RINGING!\n"; }
    bool armed() const { return armed_; }
private:
    bool armed_ = false;
};

// Concrete mediator: the only class that knows about all the devices.
class SmartHub : public Hub {
public:
    DoorSensor door{*this, "door"};
    NightButton nightButton{*this, "night button"};
    Lights lights{*this, "lights"};
    Alarm alarm{*this, "alarm"};

    void notify(const Device& sender, std::string_view event) override {
        if (&sender == &door && event == "opened") {
            if (alarm.armed()) alarm.ring();
            else               lights.turnOn();
        } else if (&sender == &nightButton && event == "pressed") {
            lights.turnOff();
            alarm.arm();
        } else {
            std::cout << "  hub: no rule for " << sender.name() << '/' << event << '\n';
        }
    }
};

int main() {
    SmartHub home;
    home.door.open();         // evening: someone comes home
    home.nightButton.press(); // bedtime
    home.door.open();         // 3 a.m.: intruder!
}
