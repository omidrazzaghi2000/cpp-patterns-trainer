// Factory Method: each logistics branch decides which transport its deliveries use.
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

// Product: the interface every transport offers.
class Transport {
public:
    virtual ~Transport() = default;
    virtual std::string name() const = 0;
    virtual int costPerKm() const = 0;
    virtual void deliver(std::string_view cargo) const = 0;
};

class Truck : public Transport {
public:
    std::string name() const override { return "truck"; }
    int costPerKm() const override { return 2; }
    void deliver(std::string_view cargo) const override {
        std::cout << "  truck drives " << cargo << " along the highway\n";
    }
};

class Ship : public Transport {
public:
    std::string name() const override { return "ship"; }
    int costPerKm() const override { return 1; }
    void deliver(std::string_view cargo) const override {
        std::cout << "  ship carries " << cargo << " across the sea\n";
    }
};

// Creator: owns the business logic but lets subclasses pick the product.
class Logistics {
public:
    virtual ~Logistics() = default;

    void planDelivery(std::string_view cargo, int km) const {
        const std::unique_ptr<Transport> transport = createTransport();  // factory method
        std::cout << "Plan: " << cargo << ", " << km << " km by "
                  << transport->name() << '\n';
        transport->deliver(cargo);
        std::cout << "  cost: " << km * transport->costPerKm() << " EUR\n";
    }

protected:
    // The factory method: "which transport?" is answered by each subclass.
    virtual std::unique_ptr<Transport> createTransport() const = 0;
};

class RoadLogistics : public Logistics {
protected:
    std::unique_ptr<Transport> createTransport() const override {
        return std::make_unique<Truck>();
    }
};

class SeaLogistics : public Logistics {
protected:
    std::unique_ptr<Transport> createTransport() const override {
        return std::make_unique<Ship>();
    }
};

// Client code talks to the abstract Creator only; it never names Truck or Ship.
void dispatch(const Logistics& branch, std::string_view cargo, int km) {
    branch.planDelivery(cargo, km);
}

int main() {
    const RoadLogistics road;
    const SeaLogistics sea;
    dispatch(road, "12 pallets of tiles", 400);
    dispatch(sea, "3 containers of cotton", 5000);
    // Adding AirLogistics + Drone later needs no change to Logistics or dispatch().
}
