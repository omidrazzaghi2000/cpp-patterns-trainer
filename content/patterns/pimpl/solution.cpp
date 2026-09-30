// Solution: Leaderboard's header now shows only a pointer to an incomplete Impl.
// The three sections below stand for three separate files.

// ===== leaderboard.h =====
#include <memory>
#include <string>

class Leaderboard {
public:
    Leaderboard();
    ~Leaderboard();
    Leaderboard(Leaderboard&&) noexcept;
    Leaderboard& operator=(Leaderboard&&) noexcept;

    void addPoints(const std::string& player, int points);
    void print() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ===== leaderboard.cpp =====  (#include "leaderboard.h")
#include <iostream>
#include <map>

struct Leaderboard::Impl {
    std::map<std::string, int> scores;
    int rounds = 0;
};

Leaderboard::Leaderboard() : impl_(std::make_unique<Impl>()) {}
Leaderboard::~Leaderboard() = default;  // stays here: Impl must be complete at this point
Leaderboard::Leaderboard(Leaderboard&&) noexcept = default;
Leaderboard& Leaderboard::operator=(Leaderboard&&) noexcept = default;

void Leaderboard::addPoints(const std::string& player, int points) {
    impl_->scores[player] += points;
    ++impl_->rounds;
}

void Leaderboard::print() const {
    std::cout << "after " << impl_->rounds << " rounds:\n";
    for (const auto& [player, score] : impl_->scores) {
        std::cout << "  " << player << ": " << score << '\n';
    }
}

// ===== main.cpp =====  (#include "leaderboard.h")
#include <utility>

int main() {
    Leaderboard board;
    board.addPoints("mira", 120);
    board.addPoints("kian", 90);
    board.addPoints("mira", 45);

    Leaderboard archive = std::move(board);  // hand the season's results to the archive
    archive.print();
    std::cout << std::boolalpha << "header exposes only a pointer: "
              << (sizeof(Leaderboard) == sizeof(void*)) << '\n';
}
