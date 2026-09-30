// Exercise: the thumbnail service allocates a fresh 8 MB canvas for every photo.
// Make CanvasPool reuse canvases, returning them automatically via the handle.
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Canvas {
public:
    explicit Canvas(int id) : id_(id) {
        std::cout << "allocating 8 MB canvas #" << id_ << '\n';
    }
    void draw(std::string_view photo) {
        std::cout << "canvas #" << id_ << (layers_.empty() ? " (blank)" : " (DIRTY!)")
                  << " -> thumbnail of " << photo << '\n';
        layers_.emplace_back(photo);
    }
    void clear() { layers_.clear(); }

private:
    int id_;
    std::vector<std::string> layers_;
};

class CanvasPool {
public:
    struct ReturnToPool {
        CanvasPool* pool;
        void operator()([[maybe_unused]] Canvas* canvas) const {
            // TODO 2: instead of doing nothing, give the canvas back: pool->release(canvas)
        }
    };
    using Handle = std::unique_ptr<Canvas, ReturnToPool>;

    CanvasPool() = default;
    CanvasPool(const CanvasPool&) = delete;
    CanvasPool& operator=(const CanvasPool&) = delete;

    Handle acquire() {
        // TODO 1: if free_ holds an idle canvas, take it (back() + pop_back())
        //         and return a Handle to it instead of allocating a new one.
        all_.push_back(std::make_unique<Canvas>(static_cast<int>(all_.size()) + 1));
        return Handle(all_.back().get(), ReturnToPool{this});
    }
    std::size_t allocated() const { return all_.size(); }

private:
    void release(Canvas* canvas) {
        // TODO 3: wipe the canvas first, so the next photo starts on a blank one.
        free_.push_back(canvas);
    }

    std::vector<std::unique_ptr<Canvas>> all_;  // the pool owns every canvas
    std::vector<Canvas*> free_;                 // idle canvases, ready for reuse
};

void makeThumbnail(CanvasPool& pool, std::string_view photo) {
    auto canvas = pool.acquire();
    canvas->draw(photo);
}  // the handle dies here: its deleter decides what happens to the canvas

int main() {
    CanvasPool pool;
    for (std::string_view photo : {"beach.jpg", "cat.png", "mountain.jpg"}) {
        makeThumbnail(pool, photo);
    }
    std::cout << "canvases allocated: " << pool.allocated() << '\n';
}
