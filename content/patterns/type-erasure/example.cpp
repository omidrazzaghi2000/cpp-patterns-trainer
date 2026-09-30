// Type Erasure: one value type that can hold any sensor, no common base class.
#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Types from different vendor SDKs: unrelated, no virtual functions, can't be edited.
struct Thermometer {
    int celsius;
    std::string reading() const { return std::to_string(celsius) + " C"; }
};
struct DoorContact {
    bool open;
    std::string reading() const { return open ? "open" : "closed"; }
};
struct AirQuality {
    int index;
    std::string reading() const { return "AQI " + std::to_string(index); }
};

template <typename T>
concept HasReading = requires(const T& t) {
    { t.reading() } -> std::convertible_to<std::string>;
};

class Sensor {
public:
    // Accepts ANY type with reading(); its concrete type is erased right here.
    template <HasReading T>
    Sensor(T device) : self_(std::make_unique<Model<T>>(std::move(device))) {}

    // Value semantics: copying a Sensor deep-copies the hidden object.
    Sensor(const Sensor& other) : self_(other.self_->clone()) {}
    Sensor& operator=(const Sensor& other) { return *this = Sensor(other); }
    Sensor(Sensor&&) noexcept = default;
    Sensor& operator=(Sensor&&) noexcept = default;

    std::string reading() const { return self_->reading(); }

private:
    struct Concept {  // the internal interface: what every sensor must offer
        virtual ~Concept() = default;
        virtual std::string reading() const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    template <typename T>
    struct Model final : Concept {  // adapts one concrete T to the Concept
        explicit Model(T d) : device(std::move(d)) {}
        std::string reading() const override { return device.reading(); }
        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model>(*this);
        }
        T device;
    };

    std::unique_ptr<Concept> self_;
};

void show(std::string_view label, const std::vector<Sensor>& sensors) {
    std::cout << label;
    for (const Sensor& s : sensors) std::cout << " [" << s.reading() << ']';
    std::cout << '\n';
}

int main() {
    // Unrelated types live in ONE container, stored by value (no pointers).
    std::vector<Sensor> home{Thermometer{21}, DoorContact{false}, AirQuality{42}};
    show("live:    ", home);

    std::vector<Sensor> snapshot = home;  // deep copy of every hidden object
    home[1] = DoorContact{true};          // the front door opens
    home.push_back(Thermometer{4});       // a fridge sensor is added later

    show("live:    ", home);
    show("snapshot:", snapshot);          // unaffected: it owns its own copies
}
