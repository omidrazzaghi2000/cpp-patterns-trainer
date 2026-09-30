// Exercise: make a text editor share character styles through a Flyweight pool.
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// Meant to be the shared flyweight (intrinsic state).
// TODO 1: `letter` is different for almost every character, so it is EXTRINSIC state.
//         Move it out of TextStyle into Glyph, and drop the `letter` parameter of get().
struct TextStyle {
    char letter;
    std::string font;
    int size;
    bool bold;
    // TODO 2: make styles comparable:   bool operator==(const TextStyle&) const = default;
};

// Flyweight factory
class StylePool {
public:
    const TextStyle& get(char letter, const std::string& font, int size, bool bold) {
        const TextStyle wanted{letter, font, size, bold};
        // TODO 3: if pool_ already holds a style equal to `wanted`, return that one instead
        //         of creating a new object (hint: std::ranges::find_if, *ptr == wanted).
        pool_.push_back(std::make_unique<const TextStyle>(wanted));
        return *pool_.back();
    }
    std::size_t size() const { return pool_.size(); }

private:
    std::vector<std::unique_ptr<const TextStyle>> pool_;
};

// Context: one per character in the document.
struct Glyph {
    const TextStyle* style;  // TODO 1: add `char letter;` as the first member
};

class Document {
public:
    explicit Document(StylePool& pool) : pool_(pool) {}

    void type(std::string_view text, const std::string& font, int size, bool bold) {
        for (char c : text) glyphs_.push_back({&pool_.get(c, font, size, bold)});
    }

    std::string text() const {
        std::string out;
        for (const Glyph& g : glyphs_) out += g.style->letter;  // TODO 1: use g.letter
        return out;
    }

    const std::vector<Glyph>& glyphs() const { return glyphs_; }

private:
    StylePool& pool_;
    std::vector<Glyph> glyphs_;
};

int main() {
    StylePool pool;
    Document doc(pool);
    doc.type("MENU ", "Serif", 18, true);
    doc.type("tea 3, cake 5, soup 4", "Sans", 11, false);

    const auto& g = doc.glyphs();
    std::cout << "text: " << doc.text() << '\n'
              << "glyphs: " << g.size() << ", style objects: " << pool.size() << '\n'
              << std::boolalpha
              << "'t' and 'c' share a style: " << (g[5].style == g[12].style) << '\n';
}
