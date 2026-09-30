// Observer: a live football match notifies every subscribed screen when a goal is scored.
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Score {
    int home = 0;
    int away = 0;
};

// Observer interface: the match only ever talks to this abstraction.
class MatchObserver {
public:
    virtual ~MatchObserver() = default;
    virtual void onGoal(const std::string& team, int minute, const Score& score) = 0;
};

// Subject: keeps a list of observers and pushes every goal to all of them.
class Match {
public:
    Match(std::string home, std::string away)
        : home_(std::move(home)), away_(std::move(away)) {}

    void subscribe(MatchObserver& o) { observers_.push_back(&o); }
    void unsubscribe(MatchObserver& o) { std::erase(observers_, &o); }  // C++20

    void goal(bool homeTeam, int minute) {
        if (homeTeam) ++score_.home; else ++score_.away;
        const std::string& team = homeTeam ? home_ : away_;
        std::cout << minute << "' GOAL " << team << " -> " << observers_.size()
                  << " observers\n";
        for (MatchObserver* o : observers_) o->onGoal(team, minute, score_);
    }

private:
    std::string home_;
    std::string away_;
    Score score_;
    std::vector<MatchObserver*> observers_;  // non-owning: unsubscribe before destruction!
};

// Concrete observers: each reacts in its own way and uses only the data it needs.
class Scoreboard : public MatchObserver {
public:
    void onGoal(const std::string&, int, const Score& score) override {
        std::cout << "  scoreboard: " << score.home << " - " << score.away << '\n';
    }
};

class FanApp : public MatchObserver {
public:
    void onGoal(const std::string& team, int minute, const Score&) override {
        std::cout << "  fan app: \"" << team << " scored in minute " << minute << "!\"\n";
    }
};

class HighlightsReel : public MatchObserver {
public:
    void onGoal(const std::string&, int minute, const Score&) override {
        std::cout << "  highlights: clip saved at " << minute << "'\n";
    }
};

int main() {
    Scoreboard board;  // observers are declared first, so they outlive the match
    FanApp app;
    HighlightsReel reel;
    Match match("Reds", "Blues");

    match.subscribe(board);
    match.subscribe(app);
    match.goal(true, 23);

    std::cout << "-- half-time: highlights joins, fan app leaves --\n";
    match.subscribe(reel);
    match.unsubscribe(app);
    match.goal(false, 67);
}
