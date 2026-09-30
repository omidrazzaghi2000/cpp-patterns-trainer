// Copy-and-Swap: exception-safe assignment for an audio clip that owns a raw buffer.
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <new>
#include <utility>

bool g_simulateOutOfMemory = false;  // lets the demo make an allocation fail

short* allocateSamples(std::size_t count) {
    if (g_simulateOutOfMemory) throw std::bad_alloc{};
    return new short[count];
}

class AudioClip {
public:
    AudioClip(std::initializer_list<short> samples)
        : size_(samples.size()), data_(allocateSamples(size_)) {
        std::copy(samples.begin(), samples.end(), data_);
    }
    ~AudioClip() { delete[] data_; }

    // Copy constructor: all the risky work (allocating) happens here.
    AudioClip(const AudioClip& other) : size_(other.size_), data_(allocateSamples(size_)) {
        std::copy(other.data_, other.data_ + size_, data_);
    }
    // Move constructor: start empty, then steal by swapping. Never allocates.
    AudioClip(AudioClip&& other) noexcept { swap(*this, other); }

    // swap only exchanges a size and a pointer, so it can never throw.
    friend void swap(AudioClip& a, AudioClip& b) noexcept {
        using std::swap;
        swap(a.size_, b.size_);
        swap(a.data_, b.data_);
    }

    // Takes its argument BY VALUE: the copy is made before *this is touched.
    // One operator serves as both copy- and move-assignment.
    AudioClip& operator=(AudioClip other) noexcept {
        swap(*this, other);
        return *this;
    }  // `other` now holds our old buffer and frees it on the way out

    friend std::ostream& operator<<(std::ostream& out, const AudioClip& clip) {
        out << '[';
        for (std::size_t i = 0; i < clip.size_; ++i) out << (i ? " " : "") << clip.data_[i];
        return out << ']';
    }

private:
    std::size_t size_ = 0;
    short* data_ = nullptr;  // raw owning pointer: exactly what this idiom manages
};

int main() {
    AudioClip drums{3, 1, 4};
    AudioClip vocals{9, 2};
    std::cout << "drums " << drums << ", vocals " << vocals << '\n';

    vocals = drums;  // copy, then swap; the old vocals buffer is released
    std::cout << "vocals = drums  -> vocals " << vocals << '\n';

    vocals = vocals;  // self-assignment: correct without any "if (this == &other)"
    std::cout << "vocals = vocals -> vocals " << vocals << '\n';

    AudioClip bass{5, 5, 5, 5};
    g_simulateOutOfMemory = true;  // from now on every allocation fails
    try {
        drums = bass;  // the copy throws before drums is modified
    } catch (const std::bad_alloc&) {
        std::cout << "drums = bass    -> failed: out of memory\n";
    }
    std::cout << "drums is intact: " << drums << '\n';

    AudioClip moved = std::move(bass);  // moving never allocates, so it still works
    std::cout << "moved bass: " << moved << '\n';
}
