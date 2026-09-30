// Composite: files and folders form a tree that clients treat uniformly.
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Component: what every node can do, whether it is a single file or a whole folder.
class Node {
public:
    explicit Node(std::string name) : name_(std::move(name)) {}
    virtual ~Node() = default;
    virtual std::size_t size() const = 0;         // in bytes
    virtual void print(int depth) const = 0;      // no default argument on virtuals!
    const std::string& name() const { return name_; }

protected:
    static std::string indent(int depth) { return std::string(depth * 2, ' '); }

private:
    std::string name_;
};

// Leaf: has no children and does the real work itself.
class File : public Node {
public:
    File(std::string name, std::size_t bytes) : Node(std::move(name)), bytes_(bytes) {}
    std::size_t size() const override { return bytes_; }
    void print(int depth) const override {
        std::cout << indent(depth) << name() << " (" << bytes_ << " B)\n";
    }

private:
    std::size_t bytes_;
};

// Composite: stores children through the SAME interface and delegates to them.
class Folder : public Node {
public:
    using Node::Node;

    Folder& add(std::unique_ptr<Node> child) {
        children_.push_back(std::move(child));
        return *this;  // allows chaining: folder.add(a).add(b)
    }

    std::size_t size() const override {
        std::size_t total = 0;
        for (const auto& child : children_) total += child->size();  // recursion
        return total;
    }

    void print(int depth) const override {
        std::cout << indent(depth) << name() << "/ (" << size() << " B)\n";
        for (const auto& child : children_) child->print(depth + 1);
    }

private:
    std::vector<std::unique_ptr<Node>> children_;  // files OR folders
};

// Client code: works with one file or an entire tree, and can't tell the difference.
void report(const Node& node) {
    std::cout << node.name() << " -> " << node.size() << " bytes\n";
}

int main() {
    auto util = std::make_unique<Folder>("util");
    util->add(std::make_unique<File>("strings.cpp", 900));

    auto src = std::make_unique<Folder>("src");
    src->add(std::make_unique<File>("main.cpp", 1200))
        .add(std::make_unique<File>("parser.cpp", 3400))
        .add(std::move(util));  // a folder inside a folder

    Folder project("project");
    project.add(std::make_unique<File>("README.md", 800)).add(std::move(src));

    project.print(0);
    File license("LICENSE", 1100);
    report(license);  // a leaf...
    report(project);  // ...and a composite, through the same call
}
