// Exercise: let every Level subclass decide which Enemy its waves are made of.
#include <iostream>
#include <memory>
#include <string>
#include <utility>

class Enemy {
public:
    virtual ~Enemy() = default;
    virtual std::string name() const = 0;
    virtual int health() const = 0;
};

class Slime : public Enemy {
public:
    std::string name() const override { return "slime"; }
    int health() const override { return 10; }
};

class Wolf : public Enemy {
public:
    std::string name() const override { return "wolf"; }
    int health() const override { return 25; }
};

class Skeleton : public Enemy {
public:
    std::string name() const override { return "skeleton"; }
    int health() const override { return 40; }
};

class Level {
public:
    explicit Level(std::string title) : title_(std::move(title)) {}
    virtual ~Level() = default;

    void spawnWave(int count) const {
        std::cout << title_ << ':';
        for (int i = 0; i < count; ++i) {
            // TODO 2: get the enemy from the factory method instead of hard-coding Slime.
            const std::unique_ptr<Enemy> enemy = std::make_unique<Slime>();
            std::cout << ' ' << enemy->name() << '(' << enemy->health() << ')';
        }
        std::cout << '\n';
    }

protected:
    // TODO 1: add the factory method
    //         virtual std::unique_ptr<Enemy> createEnemy() const
    //         with a default implementation that returns a Slime.

private:
    std::string title_;
};

class TutorialLevel : public Level {
public:
    TutorialLevel() : Level("Tutorial") {}  // keeps the default enemy
};

class ForestLevel : public Level {
public:
    ForestLevel() : Level("Forest") {}
    // TODO 3: override createEnemy() so the forest spawns wolves.
};

class CryptLevel : public Level {
public:
    CryptLevel() : Level("Crypt") {}
    // TODO 4: override createEnemy() so the crypt spawns skeletons.
};

int main() {
    const TutorialLevel tutorial;
    const ForestLevel forest;
    const CryptLevel crypt;
    tutorial.spawnWave(2);
    forest.spawnWave(2);
    crypt.spawnWave(3);
}
