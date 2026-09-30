// Visitor: HTML export and statistics added to document elements via double dispatch.
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Paragraph; struct Image; struct CodeBlock;  // the element types

// One visit() overload per element type: the set of element types is closed.
class DocVisitor {
public:
    virtual ~DocVisitor() = default;
    virtual void visit(const Paragraph& p) = 0;
    virtual void visit(const Image& img) = 0;
    virtual void visit(const CodeBlock& code) = 0;
};

struct Element {
    virtual ~Element() = default;
    virtual void accept(DocVisitor& v) const = 0;  // 1st dispatch: on the element type
};

struct Paragraph final : Element {
    explicit Paragraph(std::string t) : text(std::move(t)) {}
    // 2nd dispatch: *this is a Paragraph here, so overload resolution picks
    // visit(const Paragraph&) - of whichever visitor was passed in.
    void accept(DocVisitor& v) const override { v.visit(*this); }
    std::string text;
};

struct Image final : Element {
    explicit Image(std::string f) : file(std::move(f)) {}
    void accept(DocVisitor& v) const override { v.visit(*this); }
    std::string file;
};

struct CodeBlock final : Element {
    CodeBlock(std::string l, int n) : lang(std::move(l)), lines(n) {}
    void accept(DocVisitor& v) const override { v.visit(*this); }
    std::string lang;
    int lines;
};

// Operation 1 is a new class - not a new virtual function in every element.
class HtmlExporter final : public DocVisitor {
public:
    void visit(const Paragraph& p) override { std::cout << "<p>" << p.text << "</p>\n"; }
    void visit(const Image& img) override {
        std::cout << "<img src=\"" << img.file << "\">\n";
    }
    void visit(const CodeBlock& c) override {
        std::cout << "<pre lang=\"" << c.lang << "\">" << c.lines << " lines</pre>\n";
    }
};

// Operation 2: a visitor can accumulate state while it walks the document.
class StatsCollector final : public DocVisitor {
public:
    void visit(const Paragraph& p) override {
        words_ += 1 + static_cast<int>(std::ranges::count(p.text, ' '));
    }
    void visit(const Image&) override { ++images_; }
    void visit(const CodeBlock& c) override { codeLines_ += c.lines; }
    void report() const {
        std::cout << "words: " << words_ << ", images: " << images_
                  << ", code lines: " << codeLines_ << '\n';
    }

private:
    int words_ = 0, images_ = 0, codeLines_ = 0;
};

int main() {
    std::vector<std::unique_ptr<Element>> doc;
    doc.push_back(std::make_unique<Paragraph>("Visitors add new operations"));
    doc.push_back(std::make_unique<Image>("uml.png"));
    doc.push_back(std::make_unique<CodeBlock>("cpp", 12));
    doc.push_back(std::make_unique<Paragraph>("without editing element classes"));

    HtmlExporter html;
    for (const auto& e : doc) e->accept(html);  // same elements...

    StatsCollector stats;
    for (const auto& e : doc) e->accept(stats);  // ...a different operation
    stats.report();
}
