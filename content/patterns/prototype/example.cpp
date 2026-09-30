// Prototype: a drawing app duplicates shapes via clone(), without knowing their classes.
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Shape {
public:
    virtual ~Shape() = default;
    // The prototype interface: every shape knows how to copy itself.
    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual void draw() const = 0;

    void moveBy(int dx, int dy) { x_ += dx; y_ += dy; }

protected:
    Shape(int x, int y, std::string color) : x_(x), y_(y), color_(std::move(color)) {}
    Shape(const Shape&) = default;             // for subclasses' copy constructors
    Shape& operator=(const Shape&) = delete;   // no slicing through a Shape&

    int x_;
    int y_;
    std::string color_;
};

class Circle : public Shape {
public:
    Circle(int x, int y, int radius, std::string color)
        : Shape(x, y, std::move(color)), radius_(radius) {}

    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Circle>(*this);  // the copy constructor does the work
    }
    void draw() const override {
        std::cout << "  " << color_ << " circle r=" << radius_
                  << " at (" << x_ << ',' << y_ << ")\n";
    }

private:
    int radius_;
};

class Rectangle : public Shape {
public:
    Rectangle(int x, int y, int w, int h, std::string color)
        : Shape(x, y, std::move(color)), width_(w), height_(h) {}

    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Rectangle>(*this);
    }
    void draw() const override {
        std::cout << "  " << color_ << " rect " << width_ << 'x' << height_
                  << " at (" << x_ << ',' << y_ << ")\n";
    }

private:
    int width_;
    int height_;
};

using Selection = std::vector<std::unique_ptr<Shape>>;

// "Duplicate" (Ctrl+D): the editor sees only Shape*, never Circle or Rectangle.
Selection duplicate(const Selection& selection) {
    Selection copies;
    for (const auto& shape : selection) {
        auto copy = shape->clone();  // virtual call picks the right concrete copy
        copy->moveBy(10, 10);        // offset the copy so it doesn't hide the original
        copies.push_back(std::move(copy));
    }
    return copies;
}

int main() {
    Selection canvas;
    canvas.push_back(std::make_unique<Circle>(0, 0, 5, "red"));
    canvas.push_back(std::make_unique<Rectangle>(20, 5, 8, 4, "blue"));

    const Selection copies = duplicate(canvas);
    std::cout << "originals (unchanged):\n";
    for (const auto& shape : canvas) shape->draw();
    std::cout << "duplicates:\n";
    for (const auto& shape : copies) shape->draw();
}
