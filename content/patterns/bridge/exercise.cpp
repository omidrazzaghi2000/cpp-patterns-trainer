// Exercise: bridge remote controls (abstraction) to any device (implementation).
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

// Abstraction: should work with ANY Device, but right now it is welded to a Tv.
class RemoteControl {
public:
    // TODO 1: take a `Device&` in the constructor and store it in `device_` instead of
    //         owning a Tv. Use `device_` everywhere below.
    RemoteControl() = default;
    virtual ~RemoteControl() = default;

    void volumeUp() {
        tv_.setVolume(tv_.volume() + 10);
        report();
    }

protected:
    void report() const { std::cout << tv_.name() << " volume: " << tv_.volume() << '\n'; }

    Tv tv_;  // TODO 1: replace with   Device& device_;
};

// Refined abstraction: new features built ONLY from the Device primitives.
class AdvancedRemote : public RemoteControl {
public:
    // TODO 2: inherit the base constructor (using RemoteControl::RemoteControl;).

    void mute() {
        // TODO 3: set the device volume to 0, then call report().
    }
};

int main() {
    Tv tv;
    Radio radio;
    RemoteControl basic;       // TODO 4: pair this remote with `tv`
    AdvancedRemote advanced;   // TODO 4: pair this remote with `radio`

    basic.volumeUp();
    advanced.volumeUp();
    advanced.mute();
    std::cout << "tv=" << tv.volume() << " radio=" << radio.volume() << '\n';
}
