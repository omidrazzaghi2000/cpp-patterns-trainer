// Exercise: give the photo editor a working undo by saving and restoring Snapshot mementos.
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

    // TODO 1: capture the editor's current brightness_, contrast_ and grayscale_.
    Snapshot save() const { return Snapshot{0, 0, false}; }

    // TODO 2: copy the three values from `snapshot` back into the editor
    //         (then you can drop the [[maybe_unused]] attribute).
    void restore([[maybe_unused]] const Snapshot& snapshot) {}

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
        // TODO 3: if the stack is not empty, restore the editor from the newest
        //         snapshot (undo_.back()) and then remove it (pop_back()).
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
