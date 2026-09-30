// Strategy: a navigation app swaps its route-planning algorithm at runtime.
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

struct Route {
    int minutes;
    std::string via;
};

// Strategy interface: every travel mode plans the same trip in its own way.
class RouteStrategy {
public:
    virtual ~RouteStrategy() = default;
    virtual std::string_view mode() const = 0;
    virtual Route plan(int km) const = 0;
};

// Concrete strategies: interchangeable algorithms behind one interface.
class DrivingRoute : public RouteStrategy {
public:
    std::string_view mode() const override { return "car"; }
    Route plan(int km) const override {
        const int parking = 5;  // cars need time to find a spot
        return {km * 60 / 40 + parking, "ring road"};
    }
};

class TransitRoute : public RouteStrategy {
public:
    std::string_view mode() const override { return "bus"; }
    Route plan(int km) const override {
        const int walkAndWait = 10;  // reach the stop, then wait for the bus
        return {walkAndWait + km * 60 / 24, "bus line 7"};
    }
};

class CyclingRoute : public RouteStrategy {
public:
    std::string_view mode() const override { return "bike"; }
    Route plan(int km) const override { return {km * 60 / 15, "bike lanes"}; }
};

// Context: knows WHAT to do (navigate) and delegates HOW to its current strategy.
class Navigator {
public:
    explicit Navigator(std::unique_ptr<RouteStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    void setStrategy(std::unique_ptr<RouteStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    void navigate(std::string_view destination, int km) const {
        const Route route = strategy_->plan(km);
        std::cout << destination << " (" << km << " km) by " << strategy_->mode() << ": "
                  << route.minutes << " min via " << route.via << '\n';
    }

private:
    std::unique_ptr<RouteStrategy> strategy_;
};

int main() {
    Navigator nav(std::make_unique<DrivingRoute>());
    nav.navigate("Old Town", 6);

    nav.setStrategy(std::make_unique<TransitRoute>());  // the user taps the bus icon
    nav.navigate("Old Town", 6);

    nav.setStrategy(std::make_unique<CyclingRoute>());  // ...then the bike icon
    nav.navigate("Old Town", 6);
    nav.navigate("Harbour", 3);                         // the choice sticks
}
