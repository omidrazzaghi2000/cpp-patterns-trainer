// Adapter: plug a vendor's Fahrenheit probe into a monitor that expects Celsius sensors.
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Target: the interface our monitoring code is written against.
class TemperatureSensor {
public:
    virtual ~TemperatureSensor() = default;
    virtual std::string name() const = 0;
    virtual double celsius() const = 0;
};

// One of our own sensors: it already speaks the target interface.
class BoardSensor : public TemperatureSensor {
public:
    std::string name() const override { return "cpu-board"; }
    double celsius() const override { return 41.5; }
};

// Adaptee: a third-party driver we cannot change. Other names, other units.
class AcmeProbe {
public:
    AcmeProbe(int serial, int tenthsF) : serial_(serial), tenthsF_(tenthsF) {}
    int serialNumber() const { return serial_; }
    int readTenthsFahrenheit() const { return tenthsF_; }  // 1580 means 158.0 F

private:
    int serial_;
    int tenthsF_;  // simulated hardware reading
};

// Adapter: implements the target interface by translating calls to the adaptee.
class AcmeProbeAdapter : public TemperatureSensor {
public:
    explicit AcmeProbeAdapter(AcmeProbe probe) : probe_(std::move(probe)) {}

    std::string name() const override {
        return "acme-" + std::to_string(probe_.serialNumber());
    }
    double celsius() const override {
        const double fahrenheit = probe_.readTenthsFahrenheit() / 10.0;
        return (fahrenheit - 32.0) * 5.0 / 9.0;  // unit conversion lives here only
    }

private:
    AcmeProbe probe_;  // object adapter: it wraps the adaptee (composition)
};

// Client: knows only TemperatureSensor, so adapted probes fit right in.
void checkOverheat(const std::vector<std::unique_ptr<TemperatureSensor>>& sensors,
                   double limitC) {
    std::cout << std::fixed << std::setprecision(1);
    for (const auto& sensor : sensors) {
        const double t = sensor->celsius();
        std::cout << sensor->name() << ": " << t << " C"
                  << (t > limitC ? "  OVERHEAT" : "") << '\n';
    }
}

int main() {
    std::vector<std::unique_ptr<TemperatureSensor>> sensors;
    sensors.push_back(std::make_unique<BoardSensor>());
    sensors.push_back(std::make_unique<AcmeProbeAdapter>(AcmeProbe{7, 1580}));
    sensors.push_back(std::make_unique<AcmeProbeAdapter>(AcmeProbe{8, 1004}));
    checkOverheat(sensors, 60.0);
}
