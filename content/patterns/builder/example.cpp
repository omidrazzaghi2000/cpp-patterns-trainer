// Builder: assemble an HTTP request step by step, then validate it once in build().
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

// Product: immutable once built; only its Builder can create one.
class HttpRequest {
public:
    class Builder;

    void print() const {
        std::cout << method_ << ' ' << url_ << '\n';
        for (const auto& [name, value] : headers_)
            std::cout << "  " << name << ": " << value << '\n';
        if (!body_.empty()) std::cout << "  body: " << body_ << '\n';
        std::cout << "  timeout: " << timeoutMs_ << " ms\n";
    }

private:
    HttpRequest() = default;  // private: clients must go through the Builder
    std::string method_ = "GET";
    std::string url_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    int timeoutMs_ = 30000;
};

class HttpRequest::Builder {
public:
    explicit Builder(std::string url) { request_.url_ = std::move(url); }

    // Each step sets one part and returns *this, so the calls can be chained.
    Builder& method(std::string m) { request_.method_ = std::move(m); return *this; }
    Builder& header(std::string name, std::string value) {
        request_.headers_[std::move(name)] = std::move(value);
        return *this;
    }
    Builder& body(std::string b) { request_.body_ = std::move(b); return *this; }
    Builder& timeout(int ms) { request_.timeoutMs_ = ms; return *this; }

    // The final step checks rules that involve several parts at once.
    HttpRequest build() const {
        if (request_.method_ == "GET" && !request_.body_.empty())
            throw std::invalid_argument("a GET request cannot have a body");
        return request_;
    }

private:
    HttpRequest request_;  // the product under construction
};

// A "director": a reusable recipe that runs a fixed sequence of steps.
HttpRequest::Builder jsonPost(std::string url, std::string json) {
    HttpRequest::Builder builder(std::move(url));
    builder.method("POST")
        .header("Content-Type", "application/json")
        .body(std::move(json));
    return builder;
}

int main() {
    // Set only what you need; every other part keeps its default.
    const HttpRequest ping = HttpRequest::Builder("https://api.shop.test/health").build();
    ping.print();

    const HttpRequest order =
        jsonPost("https://api.shop.test/orders", R"({"sku":"A7","qty":2})")
            .header("Authorization", "Bearer abc123")
            .timeout(5000)
            .build();
    order.print();

    try {
        HttpRequest::Builder("https://api.shop.test/cart").body("oops").build().print();
    } catch (const std::invalid_argument& e) {
        std::cout << "rejected: " << e.what() << '\n';
    }
}
