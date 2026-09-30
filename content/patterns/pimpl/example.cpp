// Pimpl: SpellChecker's header shows one pointer; the dictionary lives in a hidden Impl.
// In a real project the three sections below are three separate files.

// ===== spell_checker.h =====  (clients include only this)
#include <memory>
#include <string>

class SpellChecker {
public:
    SpellChecker();
    ~SpellChecker();  // only declared here: Impl is still incomplete at this point
    SpellChecker(SpellChecker&&) noexcept;
    SpellChecker& operator=(SpellChecker&&) noexcept;

    void learn(const std::string& word);
    bool isKnown(const std::string& word) const;
    std::size_t vocabularySize() const;

private:
    struct Impl;                  // forward declaration: layout unknown to clients
    std::unique_ptr<Impl> impl_;  // the only data member the header reveals
};

// ===== spell_checker.cpp =====  (#include "spell_checker.h")
#include <cctype>
#include <unordered_set>  // heavy details stay out of the header

struct SpellChecker::Impl {
    std::unordered_set<std::string> words{"pattern", "pointer", "compile", "header"};

    static std::string normalize(std::string w) {
        for (char& c : w) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return w;
    }
};

SpellChecker::SpellChecker() : impl_(std::make_unique<Impl>()) {}
// Defined here, after Impl is complete, so unique_ptr's deleter can destroy it.
SpellChecker::~SpellChecker() = default;
SpellChecker::SpellChecker(SpellChecker&&) noexcept = default;
SpellChecker& SpellChecker::operator=(SpellChecker&&) noexcept = default;

void SpellChecker::learn(const std::string& word) {
    impl_->words.insert(Impl::normalize(word));
}
bool SpellChecker::isKnown(const std::string& word) const {
    return impl_->words.contains(Impl::normalize(word));
}
std::size_t SpellChecker::vocabularySize() const { return impl_->words.size(); }

// ===== main.cpp =====  (#include "spell_checker.h")
#include <iostream>
#include <utility>

int main() {
    SpellChecker checker;
    checker.learn("Pimpl");
    for (const char* word : {"Header", "pimpl", "poynter"}) {
        std::cout << word << ": " << (checker.isKnown(word) ? "ok" : "misspelled") << '\n';
    }

    SpellChecker moved = std::move(checker);  // cheap: only the pointer changes hands
    std::cout << "vocabulary: " << moved.vocabularySize() << " words\n";
    std::cout << std::boolalpha << "object is one pointer wide: "
              << (sizeof(SpellChecker) == sizeof(void*)) << '\n';
}
