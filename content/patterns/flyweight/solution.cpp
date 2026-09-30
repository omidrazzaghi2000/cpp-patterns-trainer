// Solution: glyphs keep their own letter and share a handful of pooled TextStyles.
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// Flyweight: only intrinsic state, identical for many characters.
struct TextStyle {
    std::string font;
    int size;
    bool bold;
    bool operator==(const TextStyle&) const = default;
};

// Flyweight factory
class StylePool {
public:
    const TextStyle& get(const std::string& font, int size, bool bold) {
        const TextStyle wanted{font, size, bold};
        auto it = std::ranges::find_if(pool_, [&](const auto& s) { return *s == wanted; });
        if (it != pool_.end()) return **it;  // reuse the shared object
        pool_.push_back(std::make_unique<const TextStyle>(wanted));
        return *pool_.back();
    }
    std::size_t size() const { return pool_.size(); }

private:
    std::vector<std::unique_ptr<const TextStyle>> pool_;
};

// Context: one per character in the document.
struct Glyph {
    char letter;              // extrinsic: stored per glyph
    const TextStyle* style;   // intrinsic: shared
};

class Document {
public:
    explicit Document(StylePool& pool) : pool_(pool) {}

    void type(std::string_view text, const std::string& font, int size, bool bold) {
        for (char c : text) glyphs_.push_back({c, &pool_.get(font, size, bold)});
    }

    std::string text() const {
        std::string out;
        for (const Glyph& g : glyphs_) out += g.letter;
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
