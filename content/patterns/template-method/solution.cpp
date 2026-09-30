// Solution: the template method render() fixes the skeleton; formatters fill in the steps.
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
        std::cout << header() << '\n';
        int sum = 0;
        for (const auto& e : expenses) {
            std::cout << row(e) << '\n';
            sum += e.amount;
        }
        if (showTotal()) {
            std::cout << totalLine(sum) << '\n';
        }
    }

private:
    virtual std::string header() const = 0;
    virtual std::string row(const Expense& e) const = 0;
    virtual std::string totalLine(int total) const {
        return "total: " + std::to_string(total);
    }
    virtual bool showTotal() const { return true; }  // hook with a default
};

class CsvFormatter final : public ReportFormatter {
    std::string header() const override { return "category,amount"; }
    std::string row(const Expense& e) const override {
        return e.category + "," + std::to_string(e.amount);
    }
    bool showTotal() const override { return false; }
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
