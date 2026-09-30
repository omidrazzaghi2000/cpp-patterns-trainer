// Exercise: turn the hard-coded Slugger into a host class configured by two
// policies, so ONE template yields URL slugs, search keys and headlines.
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
// TODO 4: add an UpperCase policy (same shape as LowerCase, using std::toupper).

// ---- Space policies
struct KeepSpaces {
    static char apply(char c) { return c; }
};
struct SpaceToDash {
    static char apply(char c) { return c == ' ' ? '-' : c; }
};

// TODO 1: make Slugger a template with two constrained policy parameters:
//         template <CharPolicy CasePolicy, CharPolicy SpacePolicy>
class Slugger {
public:
    static std::string make(std::string_view title) {
        std::string out;
        for (char c : title) {
            // TODO 2: pass c through CasePolicy::apply, then SpacePolicy::apply.
            out += c;
        }
        return out;
    }
};

// TODO 3: pick the policies for each configuration, e.g. Slugger<LowerCase, SpaceToDash>.
using UrlSlug = Slugger;
using SearchKey = Slugger;
using Headline = Slugger;

int main() {
    const std::string_view title = "Ten Tips For Clean Code";
    std::cout << "title:    " << title << '\n';
    std::cout << "url:      /blog/" << UrlSlug::make(title) << '\n';
    std::cout << "search:   " << SearchKey::make(title) << '\n';
    std::cout << "headline: " << Headline::make(title) << '\n';
}
