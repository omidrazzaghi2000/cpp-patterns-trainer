// Solution: Slugger is a host class; its behaviour is chosen by two policies.
#include <cctype>
#include <concepts>
#include <iostream>
#include <string>
#include <string_view>

// What every character policy must offer: a static char -> char mapping.
template <typename P>
concept CharPolicy = requires(char c) {
    { P::apply(c) } -> std::same_as<char>;
};

// ---- Case policies
struct KeepCase {
    static char apply(char c) { return c; }
};
struct LowerCase {
    static char apply(char c) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
};
struct UpperCase {
    static char apply(char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
};

// ---- Space policies
struct KeepSpaces {
    static char apply(char c) { return c; }
};
struct SpaceToDash {
    static char apply(char c) { return c == ' ' ? '-' : c; }
};

template <CharPolicy CasePolicy, CharPolicy SpacePolicy>
class Slugger {
public:
    static std::string make(std::string_view title) {
        std::string out;
        for (char c : title) {
            out += SpacePolicy::apply(CasePolicy::apply(c));
        }
        return out;
    }
};

using UrlSlug = Slugger<LowerCase, SpaceToDash>;
using SearchKey = Slugger<LowerCase, KeepSpaces>;
using Headline = Slugger<UpperCase, KeepSpaces>;

int main() {
    const std::string_view title = "Ten Tips For Clean Code";
    std::cout << "title:    " << title << '\n';
    std::cout << "url:      /blog/" << UrlSlug::make(title) << '\n';
    std::cout << "search:   " << SearchKey::make(title) << '\n';
    std::cout << "headline: " << Headline::make(title) << '\n';
}
