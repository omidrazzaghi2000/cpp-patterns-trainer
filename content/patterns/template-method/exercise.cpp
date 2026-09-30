// Exercise: finish the template method so every expense report follows the same
// skeleton - header, one row per expense, then (optionally) a total line.
#include <iostream>
#include <string>
#include <vector>

struct Expense {
    std::string category;
    int amount;
};

class ReportFormatter {
public:
    virtual ~ReportFormatter() = default;

    // The template method (non-virtual): the order of the steps lives here.
    void render(const std::vector<Expense>& expenses) const {
        // TODO 1: print header() first, then row(e) for every expense, and
        //         finally totalLine(sum) - but only if showTotal() returns true.
        for (const auto& e : expenses) {
            std::cout << row(e) << '\n';
        }
    }

private:
    virtual std::string header() const = 0;
    virtual std::string row(const Expense& e) const = 0;
    virtual std::string totalLine(int total) const {
        return "total: " + std::to_string(total);
    }
    // TODO 2: add a hook   virtual bool showTotal() const   that returns true by default.
};

class CsvFormatter final : public ReportFormatter {
    std::string header() const override { return "category,amount"; }
    std::string row(const Expense& e) const override {
        return e.category + "," + std::to_string(e.amount);
    }
    // TODO 3: spreadsheets choke on a summary line - override the hook to hide the total.
};

class MarkdownFormatter final : public ReportFormatter {
    std::string header() const override { return "| category | amount |\n|---|---|"; }
    std::string row(const Expense& e) const override {
        return "| " + e.category + " | " + std::to_string(e.amount) + " |";
    }
    std::string totalLine(int total) const override {
        return "**total: " + std::to_string(total) + "**";
    }
};

int main() {
    const std::vector<Expense> march{{"rent", 900}, {"food", 320}, {"travel", 150}};

    std::cout << "-- expenses.csv --\n";
    CsvFormatter{}.render(march);
    std::cout << "-- expenses.md --\n";
    MarkdownFormatter{}.render(march);
}
