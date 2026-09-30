// Solution: each concrete document clones itself, so nothing is sliced away.
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

class Document {
public:
    explicit Document(std::string title) : title_(std::move(title)) {}
    virtual ~Document() = default;

    // Only the concrete class knows its full type, so only it can copy itself.
    virtual std::unique_ptr<Document> clone() const = 0;

    virtual std::string describe() const { return "document '" + title_ + "'"; }
    void rename(std::string title) { title_ = std::move(title); }

protected:
    std::string title_;
};

class Invoice : public Document {
public:
    Invoice(std::string title, std::string currency, int vatPercent)
        : Document(std::move(title)), currency_(std::move(currency)), vat_(vatPercent) {}

    std::unique_ptr<Document> clone() const override {
        return std::make_unique<Invoice>(*this);
    }

    std::string describe() const override {
        return "invoice '" + title_ + "' in " + currency_
             + ", VAT " + std::to_string(vat_) + "%";
    }

private:
    std::string currency_;
    int vat_;
};

class Letter : public Document {
public:
    Letter(std::string title, std::string greeting)
        : Document(std::move(title)), greeting_(std::move(greeting)) {}

    std::unique_ptr<Document> clone() const override {
        return std::make_unique<Letter>(*this);
    }

    std::string describe() const override {
        return "letter '" + title_ + "' starting \"" + greeting_ + "\"";
    }

private:
    std::string greeting_;
};

// Prototype registry: preconfigured documents, copied on demand.
class TemplateGallery {
public:
    void add(const std::string& key, std::unique_ptr<Document> prototype) {
        prototypes_[key] = std::move(prototype);
    }
    std::unique_ptr<Document> create(const std::string& key) const {
        return prototypes_.at(key)->clone();
    }

private:
    std::map<std::string, std::unique_ptr<Document>> prototypes_;
};

int main() {
    TemplateGallery gallery;
    gallery.add("invoice", std::make_unique<Invoice>("Invoice", "EUR", 19));
    gallery.add("letter", std::make_unique<Letter>("Letter", "Dear customer,"));

    auto march = gallery.create("invoice");
    march->rename("INV-0301");
    auto welcome = gallery.create("letter");
    welcome->rename("Welcome pack");

    std::cout << march->describe() << '\n';
    std::cout << welcome->describe() << '\n';
    std::cout << "template kept: " << gallery.create("invoice")->describe() << '\n';
}
