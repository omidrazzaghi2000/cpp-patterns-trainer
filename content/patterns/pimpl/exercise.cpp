// Exercise: hide Leaderboard's data behind a Pimpl so its header exposes only a pointer.
// The three sections below stand for three separate files.

// ===== leaderboard.h =====
#include <map>  // clients should not need this - see TODO 3
#include <memory>
#include <string>

class Leaderboard {
public:
    Leaderboard();
    ~Leaderboard();
    // TODO 1: declare the move constructor and move assignment (both noexcept).
    //         A user-declared destructor suppresses the implicit ones.

    void addPoints(const std::string& player, int points);
    void print() const;

private:
    // TODO 2: replace these two members with   struct Impl;   and
    //         std::unique_ptr<Impl> impl_;
    std::map<std::string, int> scores_;
    int rounds_ = 0;
};

// ===== leaderboard.cpp =====  (#include "leaderboard.h")
#include <iostream>

// TODO 3: define   struct Leaderboard::Impl   here with the scores and the round
//         counter (and move #include <map> down to this section).

// TODO 4: create the Impl with std::make_unique, and define the two move
//         operations as "= default" next to the destructor.
Leaderboard::Leaderboard() = default;
Leaderboard::~Leaderboard() = default;  // stays here: Impl must be complete at this point

// TODO 5: reach the data through impl_-> in both member functions.
void Leaderboard::addPoints(const std::string& player, int points) {
    scores_[player] += points;
    ++rounds_;
}

void Leaderboard::print() const {
    std::cout << "after " << rounds_ << " rounds:\n";
    for (const auto& [player, score] : scores_) {
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
