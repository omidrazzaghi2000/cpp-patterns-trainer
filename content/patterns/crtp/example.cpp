// CRTP: zero-overhead static "interfaces" for sensors, plus a per-type counting mixin.
#include <iostream>
#include <string_view>

// Static interface: Sensor<Derived> knows the concrete type at compile time, so
// derived().raw() is an ordinary, inlinable call - no virtual, no vtable.
template <typename Derived>
class Sensor {
public:
    double read() const {
        const double value = derived().raw() * derived().scale();
        std::cout << derived().name() << ": " << value << ' ' << derived().unit() << '\n';
        return value;
    }

private:
    Sensor() = default;  // private + friend: only Derived can construct this base, so
    friend Derived;      // a typo like `class Hygro : Sensor<Barometer>` won't compile
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

// Mixin: gives every class that inherits it its own live-object counter.
template <typename T>
class Counted {
public:
    static int alive() { return count_; }

protected:
    Counted() { ++count_; }
    Counted(const Counted&) { ++count_; }
    Counted& operator=(const Counted&) = default;
    ~Counted() { --count_; }  // protected: nobody deletes through Counted<T>*

private:
    inline static int count_ = 0;  // Counted<A> and Counted<B> are distinct classes
};

class Thermometer : public Sensor<Thermometer>, public Counted<Thermometer> {
public:
    int raw() const { return 180; }  // e.g. an ADC register value
    double scale() const { return 0.125; }
    std::string_view name() const { return "thermometer"; }
    std::string_view unit() const { return "C"; }
};

class Barometer : public Sensor<Barometer>, public Counted<Barometer> {
public:
    int raw() const { return 4052; }
    double scale() const { return 0.25; }
    std::string_view name() const { return "barometer"; }
    std::string_view unit() const { return "hPa"; }
};

// One generic function for every Sensor<T>; each call is resolved at compile time.
template <typename T>
void logReading(const Sensor<T>& sensor) {
    sensor.read();
}

int main() {
    const Thermometer kitchen;
    const Barometer roof;
    logReading(kitchen);
    logReading(roof);

    {
        const Thermometer attic, cellar;
        std::cout << "thermometers alive: " << Thermometer::alive() << '\n';
    }
    std::cout << "thermometers alive: " << Thermometer::alive() << '\n';
    std::cout << "barometers alive: " << Barometer::alive() << '\n';
    std::cout << "sizeof(Thermometer): " << sizeof(Thermometer) << " (no vtable pointer)\n";
}
