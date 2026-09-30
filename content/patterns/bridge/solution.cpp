// Solution: remotes hold a Device& bridge, so any remote works with any device.
#include <iostream>
#include <string>

// Implementation interface: only the primitive operations every device supports.
class Device {
public:
    virtual ~Device() = default;
    virtual std::string name() const = 0;
    virtual int volume() const = 0;
    virtual void setVolume(int percent) = 0;
};

class Tv : public Device {
public:
    std::string name() const override { return "TV"; }
    int volume() const override { return volume_; }
    void setVolume(int percent) override { volume_ = percent; }

private:
    int volume_ = 30;
};

class Radio : public Device {
public:
    std::string name() const override { return "Radio"; }
    int volume() const override { return volume_; }
    void setVolume(int percent) override { volume_ = percent; }

private:
    int volume_ = 20;
};

// Abstraction: talks to the Device interface only; the device is chosen by the caller.
class RemoteControl {
public:
    explicit RemoteControl(Device& device) : device_(device) {}
    virtual ~RemoteControl() = default;

    void volumeUp() {
        device_.setVolume(device_.volume() + 10);
        report();
    }

protected:
    void report() const {
        std::cout << device_.name() << " volume: " << device_.volume() << '\n';
    }

    Device& device_;  // the bridge: a remote does not own the device it controls
};

// Refined abstraction: new features built ONLY from the Device primitives.
class AdvancedRemote : public RemoteControl {
public:
    using RemoteControl::RemoteControl;

    void mute() {
        device_.setVolume(0);
        report();
    }
};

int main() {
    Tv tv;
    Radio radio;
    RemoteControl basic(tv);
    AdvancedRemote advanced(radio);

    basic.volumeUp();
    advanced.volumeUp();
    advanced.mute();
    std::cout << "tv=" << tv.volume() << " radio=" << radio.volume() << '\n';
}
