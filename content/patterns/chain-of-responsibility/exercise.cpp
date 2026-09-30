// Exercise: route purchase requests up an approval chain until someone may approve them.
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Purchase {
    std::string item;
    int amount;  // in dollars
};

// Every approver has a spending limit and (maybe) a next approver above them.
class Approver {
public:
    Approver(std::string title, int limit) : title_(std::move(title)), limit_(limit) {}

    // Links `next` after this approver and returns it, so links can be chained.
    Approver& then(std::unique_ptr<Approver> next) {
        next_ = std::move(next);
        return *next_;
    }

    void review(const Purchase& p) const {
        // TODO 1: approve only if p.amount <= limit_ (print the line below and stop).
        // TODO 2: otherwise, if there is a next_ approver, forward the purchase to it.
        // TODO 3: if nobody is left, print "<item> ($<amount>) rejected: over every limit".
        std::cout << title_ << " approved " << p.item << " ($" << p.amount << ")\n";
    }

private:
    std::string title_;
    int limit_;
    std::unique_ptr<Approver> next_;
};

int main() {
    auto chain = std::make_unique<Approver>("TeamLead", 1'000);
    // TODO 4: link a "Manager" (limit 10'000) and then a "CFO" (limit 50'000)
    //         after the TeamLead, using then().

    const std::vector<Purchase> requests{
        {"keyboard", 120}, {"laptop", 2'400}, {"servers", 36'000}, {"warehouse", 900'000}};
    for (const Purchase& p : requests) {
        chain->review(p);  // always start at the bottom of the chain
    }
}
