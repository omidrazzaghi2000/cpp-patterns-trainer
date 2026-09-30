// Exercise: give a game inventory an iterator that skips the empty slots.
#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

struct Item {
    std::string name;
    int count = 0;  // 0 means the slot is empty
};

class Inventory {
public:
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Item;
        using difference_type = std::ptrdiff_t;
        using pointer = const Item*;
        using reference = const Item&;

        const_iterator() = default;
        const_iterator(const Inventory* inv, std::size_t slot) : inv_(inv), slot_(slot) {
            // TODO 1: call skipEmpty() so begin() lands on the first occupied slot.
        }

        reference operator*() const { return inv_->slots_[slot_]; }
        pointer operator->() const { return &inv_->slots_[slot_]; }
        const_iterator& operator++() {
            ++slot_;
            // TODO 2: after advancing, skip any empty slots as well.
            return *this;
        }
        const_iterator operator++(int) { const_iterator old = *this; ++*this; return old; }
        bool operator==(const const_iterator&) const = default;

    private:
        void skipEmpty() {
            // TODO 3: while slot_ is still inside the inventory (< inv_->slots_.size())
            //         and that slot's count is 0, move on to the next slot.
        }

        const Inventory* inv_ = nullptr;
        std::size_t slot_ = 0;
    };

    void put(std::size_t slot, std::string name, int count) {
        slots_.at(slot) = Item{std::move(name), count};
    }
    const_iterator begin() const { return {this, 0}; }
    const_iterator end() const { return {this, slots_.size()}; }

private:
    std::array<Item, 8> slots_{};  // fixed slots, most of them empty
};

static_assert(std::forward_iterator<Inventory::const_iterator>);

int main() {
    Inventory bag;
    bag.put(1, "potion", 3);
    bag.put(4, "arrow", 20);
    bag.put(5, "map", 1);
    bag.put(7, "gold", 150);

    for (const Item& item : bag) {  // goal: empty slots never show up here
        std::cout << "- " << item.name << " x" << item.count << '\n';
    }

    std::cout << "kinds of items: " << std::distance(bag.begin(), bag.end()) << '\n';
    auto isMap = [](const Item& i) { return i.name == "map"; };
    std::cout << std::boolalpha << "has a map: " << std::ranges::any_of(bag, isMap) << '\n';
}
