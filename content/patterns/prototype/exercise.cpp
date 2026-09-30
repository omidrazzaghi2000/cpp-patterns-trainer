// Exercise: fix the template gallery so new documents are FULL copies of a prototype.
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>

class Document {
public:
    explicit Document(std::string title) : title_(std::move(title)) {}
    virtual ~Document() = default;

    // TODO 1: make clone() pure virtual (= 0). This base version only copies the
    //         Document part of an object - the subclass data is "sliced" away.
    virtual std::unique_ptr<Document> clone() const {
        return std::make_unique<Document>(*this);
    }

    virtual std::string describe() const { return "document '" + title_ + "'"; }
    void rename(std::string title) { title_ = std::move(title); }

protected:
    std::string title_;
};

class Invoice : public Document {
public:
    Invoice(std::string title, std::string currency, int vatPercent)
        : Document(std::move(title)), currency_(std::move(currency)), vat_(vatPercent) {}

    // TODO 2: override clone() so it returns a copy of the whole Invoice.

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

    // TODO 3: override clone() so it returns a copy of the whole Letter.

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
