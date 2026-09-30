// Solution: PhotoEditor saves Snapshot mementos and History uses them to undo edits.
#include <iostream>
#include <string_view>
#include <vector>

// Memento: only PhotoEditor can create a Snapshot or read what is inside.
class Snapshot {
private:
    friend class PhotoEditor;
    Snapshot(int brightness, int contrast, bool grayscale)
        : brightness_(brightness), contrast_(contrast), grayscale_(grayscale) {}

    int brightness_;
    int contrast_;
    bool grayscale_;
};

// Originator.
class PhotoEditor {
public:
    void adjust(int brightness, int contrast) {
        brightness_ += brightness;
        contrast_ += contrast;
    }
    void toggleGrayscale() { grayscale_ = !grayscale_; }
    void print(std::string_view step) const {
        std::cout << step << ": brightness " << brightness_ << ", contrast " << contrast_
                  << (grayscale_ ? ", grayscale" : ", color") << '\n';
    }

    Snapshot save() const { return Snapshot{brightness_, contrast_, grayscale_}; }

    void restore(const Snapshot& snapshot) {
        brightness_ = snapshot.brightness_;
        contrast_ = snapshot.contrast_;
        grayscale_ = snapshot.grayscale_;
    }

private:
    int brightness_ = 0;
    int contrast_ = 0;
    bool grayscale_ = false;
};

// Caretaker: stores snapshots on an undo stack but never looks inside them.
class History {
public:
    explicit History(PhotoEditor& editor) : editor_(editor) {}

    void backup() { undo_.push_back(editor_.save()); }

    void undo() {
        if (undo_.empty()) return;
        editor_.restore(undo_.back());
        undo_.pop_back();
    }

private:
    PhotoEditor& editor_;
    std::vector<Snapshot> undo_;
};

int main() {
    PhotoEditor photo;
    History history(photo);
    photo.print("original");

    history.backup();
    photo.adjust(+20, +10);
    photo.print("brighter");

    history.backup();
    photo.toggleGrayscale();
    photo.adjust(-5, +30);
    photo.print("dramatic");

    history.undo();
    photo.print("undo");
    history.undo();
    photo.print("undo");
}
