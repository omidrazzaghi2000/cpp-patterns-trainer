// Command: playlist edits become objects, so every edit can be undone.
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Receiver: does the real work but knows nothing about undo.
class Playlist {
public:
    void insert(std::size_t pos, std::string track) {
        tracks_.insert(tracks_.begin() + static_cast<std::ptrdiff_t>(pos), std::move(track));
    }
    std::string removeAt(std::size_t pos) {
        std::string track = std::move(tracks_.at(pos));
        tracks_.erase(tracks_.begin() + static_cast<std::ptrdiff_t>(pos));
        return track;
    }
    std::size_t size() const { return tracks_.size(); }
    void print() const {
        std::cout << "  playlist:";
        for (const auto& t : tracks_) std::cout << " [" << t << ']';
        std::cout << '\n';
    }
private:
    std::vector<std::string> tracks_;
};

// Command: one request packaged with everything needed to perform AND reverse it.
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string label() const = 0;
};

class AddTrack : public Command {
public:
    AddTrack(Playlist& list, std::string track) : list_(list), track_(std::move(track)) {}
    void execute() override { pos_ = list_.size(); list_.insert(pos_, track_); }
    void undo() override { list_.removeAt(pos_); }
    std::string label() const override { return "add " + track_; }
private:
    Playlist& list_;
    std::string track_;
    std::size_t pos_ = 0;  // where it went, so undo knows what to remove
};

class RemoveTrack : public Command {
public:
    RemoveTrack(Playlist& list, std::size_t pos) : list_(list), pos_(pos) {}
    void execute() override { removed_ = list_.removeAt(pos_); }  // remember the victim
    void undo() override { list_.insert(pos_, removed_); }
    std::string label() const override { return "remove " + removed_; }
private:
    Playlist& list_;
    std::size_t pos_;
    std::string removed_;
};

// Invoker: runs commands and keeps the history; it never touches Playlist itself.
class EditHistory {
public:
    void run(std::unique_ptr<Command> cmd) {
        cmd->execute();
        std::cout << "do   " << cmd->label() << '\n';
        done_.push_back(std::move(cmd));  // the history owns every executed command
    }
    void undo() {
        if (done_.empty()) return;  // nothing to undo
        done_.back()->undo();
        std::cout << "undo " << done_.back()->label() << '\n';
        done_.pop_back();
    }
private:
    std::vector<std::unique_ptr<Command>> done_;
};

int main() {
    Playlist roadTrip;
    EditHistory history;
    history.run(std::make_unique<AddTrack>(roadTrip, "So What"));
    history.run(std::make_unique<AddTrack>(roadTrip, "Take Five"));
    history.run(std::make_unique<RemoveTrack>(roadTrip, 0));
    roadTrip.print();
    history.undo();  // "So What" goes back to position 0
    history.undo();  // "Take Five" disappears again
    roadTrip.print();
}
