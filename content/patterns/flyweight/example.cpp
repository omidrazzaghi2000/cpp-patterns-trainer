// Flyweight: 9,000 trees in a game map share just three heavy TreeType objects.
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Flyweight: intrinsic state only. Immutable, so it is safe to share.
class TreeType {
public:
    TreeType(std::string species, std::string color, std::size_t textureKb)
        : species_(std::move(species)), color_(std::move(color)), textureKb_(textureKb) {}

    // Extrinsic state (the position) is passed in; it is never stored here.
    void draw(int x, int y) const {
        std::cout << species_ << " (" << color_ << ") at " << x << ',' << y << '\n';
    }
    std::size_t textureKb() const { return textureKb_; }

private:
    std::string species_;
    std::string color_;
    std::size_t textureKb_;  // stands in for the big mesh + texture data
};

// Flyweight factory: hands out an existing TreeType, or creates it exactly once.
class TreeTypeFactory {
public:
    const TreeType& get(const std::string& species, const std::string& color, std::size_t kb) {
        const std::string key = species + '/' + color;
        auto it = pool_.find(key);
        if (it == pool_.end()) {
            std::cout << "[factory] loading " << key << '\n';
            auto type = std::make_unique<const TreeType>(species, color, kb);
            it = pool_.emplace(key, std::move(type)).first;
        }
        return *it->second;
    }
    std::size_t count() const { return pool_.size(); }
    std::size_t totalKb() const {
        std::size_t kb = 0;
        for (const auto& [key, type] : pool_) kb += type->textureKb();
        return kb;
    }

private:
    std::map<std::string, std::unique_ptr<const TreeType>> pool_;
};

// Context: the tiny per-tree part — extrinsic state plus a pointer to the shared type.
struct Tree {
    int x, y;
    const TreeType* type;
};

struct Species { const char* name; const char* color; std::size_t textureKb; };

int main() {
    const Species kinds[] = {{"oak", "green", 800}, {"pine", "dark green", 600},
                             {"birch", "white", 500}};
    TreeTypeFactory factory;
    std::vector<Tree> forest;
    forest.reserve(9000);
    for (int i = 0; i < 9000; ++i) {
        const Species& s = kinds[i % 3];
        const TreeType& shared = factory.get(s.name, s.color, s.textureKb);
        forest.push_back({i * 7 % 1000, i * 13 % 1000, &shared});  // ~16 bytes per tree
    }

    for (int i = 0; i < 3; ++i) forest[i].type->draw(forest[i].x, forest[i].y);

    std::size_t naiveKb = 0;  // what it would cost if every tree had its own copy
    for (const Tree& t : forest) naiveKb += t.type->textureKb();

    std::cout << std::boolalpha
              << "trees #0 and #3 share a type: " << (forest[0].type == forest[3].type) << '\n'
              << "trees: " << forest.size() << ", tree types: " << factory.count() << '\n'
              << "texture memory without sharing: " << naiveKb << " KB\n"
              << "texture memory with sharing: " << factory.totalKb() << " KB\n";
}
