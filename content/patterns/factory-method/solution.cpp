// Solution: spawnWave() calls the createEnemy() factory method; subclasses override it.
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
            const std::unique_ptr<Enemy> enemy = createEnemy();
            std::cout << ' ' << enemy->name() << '(' << enemy->health() << ')';
        }
        std::cout << '\n';
    }

protected:
    // The factory method, with a sensible default that subclasses may override.
    virtual std::unique_ptr<Enemy> createEnemy() const {
        return std::make_unique<Slime>();
    }

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

protected:
    std::unique_ptr<Enemy> createEnemy() const override {
        return std::make_unique<Wolf>();
    }
};

class CryptLevel : public Level {
public:
    CryptLevel() : Level("Crypt") {}

protected:
    std::unique_ptr<Enemy> createEnemy() const override {
        return std::make_unique<Skeleton>();
    }
};

int main() {
    const TutorialLevel tutorial;
    const ForestLevel forest;
    const CryptLevel crypt;
    tutorial.spawnWave(2);
    forest.spawnWave(2);
    crypt.spawnWave(3);
}
