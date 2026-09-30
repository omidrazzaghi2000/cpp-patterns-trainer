// Proxy: a lazy, caching stand-in for a slow stock-quote web service.
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>

// Subject: the interface shared by the real service and its proxy.
class QuoteService {
public:
    virtual ~QuoteService() = default;
    virtual double price(std::string_view symbol) = 0;
};

// RealSubject: expensive to create and every call is a network round trip.
class RemoteQuoteService : public QuoteService {
public:
    RemoteQuoteService() { std::cout << "  [remote] connecting to exchange...\n"; }

    double price(std::string_view symbol) override {
        std::cout << "  [remote] GET /quote/" << symbol << '\n';
        return symbol == "ACME" ? 42.5 : 17.25;
    }
};

// Proxy: same interface, but it decides WHEN (and whether) to bother the real object.
class CachingQuoteProxy : public QuoteService {
public:
    double price(std::string_view symbol) override {
        ++requests_;
        if (auto it = cache_.find(symbol); it != cache_.end()) {
            std::cout << "  [proxy] cache hit " << symbol << '\n';
            return it->second;
        }
        if (!real_) {
            real_ = std::make_unique<RemoteQuoteService>();  // virtual proxy: create late
        }
        const double value = real_->price(symbol);  // delegate to the real subject
        cache_.emplace(symbol, value);
        return value;
    }

    int requests() const { return requests_; }

private:
    std::unique_ptr<RemoteQuoteService> real_;          // empty until first needed
    std::map<std::string, double, std::less<>> cache_;  // less<> allows string_view lookup
    int requests_ = 0;
};

// Client: written against QuoteService, it cannot tell a proxy from the real thing.
void showPortfolio(QuoteService& quotes) {
    for (std::string_view symbol : {"ACME", "GLOBEX"}) {
        const double value = quotes.price(symbol);
        std::cout << symbol << " = " << value << '\n';
    }
}

int main() {
    CachingQuoteProxy quotes;
    std::cout << "app started, no connection yet\n";

    std::cout << "first refresh:\n";
    showPortfolio(quotes);
    std::cout << "second refresh:\n";
    showPortfolio(quotes);

    std::cout << "requests served: " << quotes.requests() << '\n';
}
