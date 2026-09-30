// Solution: Route uses copy-and-swap, so a failed reroute leaves it unchanged.
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

    friend void swap(Route& a, Route& b) noexcept {
        using std::swap;
        swap(a.count_, b.count_);
        swap(a.stops_, b.stops_);
    }

    // The copy (which may throw) is made before we get here.
    Route& operator=(Route other) noexcept {
        swap(*this, other);
        return *this;
    }

    Route(Route&& other) noexcept { swap(*this, other); }

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
