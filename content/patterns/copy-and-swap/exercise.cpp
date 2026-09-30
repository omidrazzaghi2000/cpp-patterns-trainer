// Exercise: when a reroute fails, Route's assignment operator loses the old
// route. Rewrite it with copy-and-swap to get the strong exception guarantee.
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <new>
#include <utility>

bool g_simulateOutOfMemory = false;  // lets main() make an allocation fail

int* allocateStops(std::size_t count) {
    if (g_simulateOutOfMemory) throw std::bad_alloc{};
    return new int[count];
}

class Route {
public:
    Route(std::initializer_list<int> stops)
        : count_(stops.size()), stops_(allocateStops(count_)) {
        std::copy(stops.begin(), stops.end(), stops_);
    }
    ~Route() { delete[] stops_; }

    Route(const Route& other) : count_(other.count_), stops_(allocateStops(count_)) {
        std::copy(other.stops_, other.stops_ + count_, stops_);
    }

    // TODO 1: add   friend void swap(Route& a, Route& b) noexcept
    //         that swaps count_ and stops_ (using std::swap on each member).

    // TODO 2: replace this operator with copy-and-swap: take the parameter
    //         BY VALUE, swap(*this, other), return *this. Mark it noexcept.
    Route& operator=(const Route& other) {
        if (this == &other) return *this;
        delete[] stops_;  // the old route is destroyed first...
        stops_ = nullptr;
        count_ = 0;
        stops_ = allocateStops(other.count_);  // ...then this throws: route lost!
        count_ = other.count_;
        std::copy(other.stops_, other.stops_ + count_, stops_);
        return *this;
    }

    // TODO 3: add a noexcept move constructor that starts empty and swaps
    //         with `other`, so moving never needs to allocate.

    friend std::ostream& operator<<(std::ostream& out, const Route& r) {
        out << '[';
        for (std::size_t i = 0; i < r.count_; ++i) out << (i ? " " : "") << r.stops_[i];
        return out << ']';
    }

private:
    std::size_t count_ = 0;
    int* stops_ = nullptr;
};

int main() {
    Route current{12, 7, 30};
    std::cout << "current:  " << current << '\n';
    Route detour{12, 44, 45, 30};
    current = detour;  // the normal case works either way
    std::cout << "rerouted: " << current << '\n';

    Route scenic{12, 90, 91, 30};
    g_simulateOutOfMemory = true;  // the phone runs out of memory
    try {
        current = scenic;
    } catch (const std::bad_alloc&) {
        std::cout << "reroute failed: out of memory\n";
    }
    std::cout << "still navigating: " << current << '\n';

    try {
        Route guidance = std::move(current);  // hand the route to voice guidance
        std::cout << "voice guidance got: " << guidance << '\n';
    } catch (const std::bad_alloc&) {
        std::cout << "handover failed: out of memory\n";
    }
}
