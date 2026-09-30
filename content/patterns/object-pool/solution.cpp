// Solution: CanvasPool reuses idle canvases; the handle's deleter returns them.
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
        void operator()(Canvas* canvas) const { pool->release(canvas); }
    };
    using Handle = std::unique_ptr<Canvas, ReturnToPool>;

    CanvasPool() = default;
    CanvasPool(const CanvasPool&) = delete;
    CanvasPool& operator=(const CanvasPool&) = delete;

    Handle acquire() {
        if (!free_.empty()) {  // reuse an idle canvas: no allocation
            Canvas* canvas = free_.back();
            free_.pop_back();
            return Handle(canvas, ReturnToPool{this});
        }
        all_.push_back(std::make_unique<Canvas>(static_cast<int>(all_.size()) + 1));
        return Handle(all_.back().get(), ReturnToPool{this});
    }
    std::size_t allocated() const { return all_.size(); }

private:
    void release(Canvas* canvas) {
        canvas->clear();  // no leftovers from the previous photo
        free_.push_back(canvas);
    }

    std::vector<std::unique_ptr<Canvas>> all_;  // the pool owns every canvas
    std::vector<Canvas*> free_;                 // idle canvases, ready for reuse
};

void makeThumbnail(CanvasPool& pool, std::string_view photo) {
    auto canvas = pool.acquire();
    canvas->draw(photo);
}  // the handle dies here: its deleter puts the canvas back in the pool

int main() {
    CanvasPool pool;
    for (std::string_view photo : {"beach.jpg", "cat.png", "mountain.jpg"}) {
        makeThumbnail(pool, photo);
    }
    std::cout << "canvases allocated: " << pool.allocated() << '\n';
}
