// Iterator: a ring buffer whose iterator hides the wrap-around, so range-for and
// standard algorithms work on it like on any STL container.
#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <iterator>

// Keeps the last N sensor readings; when full, the oldest one is overwritten.
template <std::size_t N>
class RingBuffer {
public:
    class const_iterator {
    public:
        // The five aliases std::iterator_traits and the C++20 concepts look for.
        using iterator_category = std::forward_iterator_tag;
        using value_type = double;
        using difference_type = std::ptrdiff_t;
        using pointer = const double*;
        using reference = const double&;

        const_iterator() = default;  // forward iterators must be default-constructible
        const_iterator(const RingBuffer* buf, std::size_t pos) : buf_(buf), pos_(pos) {}

        // The traversal logic lives HERE: logical position -> physical slot.
        reference operator*() const { return buf_->data_[(buf_->head_ + pos_) % N]; }
        const_iterator& operator++() { ++pos_; return *this; }
        const_iterator operator++(int) { const_iterator old = *this; ++pos_; return old; }
        bool operator==(const const_iterator&) const = default;

    private:
        const RingBuffer* buf_ = nullptr;
        std::size_t pos_ = 0;  // 0 = oldest reading
    };

    void push(double value) {
        data_[(head_ + size_) % N] = value;
        if (size_ < N) ++size_;
        else head_ = (head_ + 1) % N;  // full: the oldest slot was just overwritten
    }

    const_iterator begin() const { return {this, 0}; }
    const_iterator end() const { return {this, size_}; }  // one past the newest
    std::size_t size() const { return size_; }

private:
    std::array<double, N> data_{};
    std::size_t head_ = 0;  // physical index of the oldest reading
    std::size_t size_ = 0;
};

// Compile-time proof that the iterator models the C++20 concept.
static_assert(std::forward_iterator<RingBuffer<4>::const_iterator>);

int main() {
    RingBuffer<4> temps;
    for (double t : {21.5, 22.0, 23.5, 22.5, 24.0, 21.0}) temps.push(t);

    std::cout << "last " << temps.size() << " readings:";
    for (double t : temps) std::cout << ' ' << t;  // range-for calls begin()/end()
    std::cout << '\n';

    // Classic iterator-pair algorithm...
    auto hottest = std::max_element(temps.begin(), temps.end());
    std::cout << "max: " << *hottest << '\n';

    // ...and a C++20 range algorithm: both only see the iterator interface.
    auto warm = std::ranges::count_if(temps, [](double t) { return t > 22.0; });
    std::cout << "above 22: " << warm << '\n';
}
