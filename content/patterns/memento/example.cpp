// Memento: a game hero saves checkpoints and reloads them without exposing its internals.
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// The memento: an opaque snapshot. Only Hero can create it or read its contents.
class Checkpoint {
public:
    const std::string& label() const { return label_; }  // the only thing outsiders see

private:
    friend class Hero;
    Checkpoint(std::string label, std::string zone, int health, int gold)
        : label_(std::move(label)), zone_(std::move(zone)), health_(health), gold_(gold) {}

    std::string label_;
    std::string zone_;
    int health_;
    int gold_;
};

// The originator: its state stays private, yet it can be saved and restored.
class Hero {
public:
    Checkpoint save(std::string label) const {
        return Checkpoint{std::move(label), zone_, health_, gold_};
    }
    void restore(const Checkpoint& cp) {
        zone_ = cp.zone_;
        health_ = cp.health_;
        gold_ = cp.gold_;
    }

    void enter(std::string zone) { zone_ = std::move(zone); }
    void fight(int damage, int loot) {
        health_ = std::max(0, health_ - damage);
        gold_ += loot;
    }
    void print() const {
        std::cout << "  hero in " << zone_ << ": hp " << health_ << ", gold " << gold_ << '\n';
    }

private:
    std::string zone_ = "Village";
    int health_ = 100;
    int gold_ = 0;
};

// The caretaker: keeps checkpoints in order, but never looks inside them.
class SaveSlots {
public:
    void store(Checkpoint cp) {
        std::cout << "saved \"" << cp.label() << "\"\n";
        slots_.push_back(std::move(cp));
    }
    const Checkpoint& latest() const { return slots_.back(); }
    std::size_t count() const { return slots_.size(); }

private:
    std::vector<Checkpoint> slots_;
};

int main() {
    Hero hero;
    SaveSlots slots;
    slots.store(hero.save("new game"));

    hero.enter("Dark Forest");
    hero.fight(30, 50);
    hero.print();
    slots.store(hero.save("forest cleared"));

    hero.enter("Dragon Lair");
    hero.fight(120, 0);
    hero.print();

    std::cout << "game over! " << slots.count() << " checkpoints available\n";
    const Checkpoint& cp = slots.latest();
    std::cout << "loading \"" << cp.label() << "\"\n";
    hero.restore(cp);
    hero.print();
    // cp.health_ = 999;  // compile error: outsiders cannot read or tamper with a snapshot
}
