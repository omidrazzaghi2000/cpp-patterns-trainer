// Chain of Responsibility: support tickets escalate until someone can handle them.
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Ticket {
    std::string topic;  // "password", "billing", "outage", ...
    int severity;       // 1 (minor) .. 5 (critical)
};

// Handler: knows only the NEXT link, never the whole chain.
class SupportHandler {
public:
    virtual ~SupportHandler() = default;

    // Returns the new link so a chain can be built in one expression.
    SupportHandler& setNext(std::unique_ptr<SupportHandler> next) {
        next_ = std::move(next);
        return *next_;
    }

    // Default behaviour: pass the ticket on, or report that the chain has ended.
    virtual void handle(const Ticket& t) const {
        if (next_) {
            next_->handle(t);
        } else {
            std::cout << "  (end of chain) logged for manual review\n";
        }
    }

private:
    std::unique_ptr<SupportHandler> next_;
};

// Concrete handlers: each one either resolves the ticket or forwards it.
class FaqBot : public SupportHandler {
public:
    void handle(const Ticket& t) const override {
        if (t.topic == "password") {
            std::cout << "  FaqBot: sent the reset-password guide\n";
            return;  // handled: the request stops here
        }
        std::cout << "  FaqBot: not in my FAQ\n";
        SupportHandler::handle(t);  // forward to the next link
    }
};

class HelpDesk : public SupportHandler {
public:
    void handle(const Ticket& t) const override {
        if (t.severity <= 3) {
            std::cout << "  HelpDesk: solved the " << t.topic << " issue\n";
            return;
        }
        std::cout << "  HelpDesk: too severe for me\n";
        SupportHandler::handle(t);
    }
};

class OnCallEngineer : public SupportHandler {
public:
    void handle(const Ticket& t) const override {
        if (t.topic == "outage") {
            std::cout << "  Engineer: paged, restoring service\n";
            return;
        }
        std::cout << "  Engineer: not a technical problem\n";
        SupportHandler::handle(t);
    }
};

int main() {
    // Build the chain: FaqBot -> HelpDesk -> OnCallEngineer.
    auto chain = std::make_unique<FaqBot>();
    chain->setNext(std::make_unique<HelpDesk>())
          .setNext(std::make_unique<OnCallEngineer>());

    const std::vector<Ticket> inbox{
        {"password", 1}, {"billing", 2}, {"outage", 5}, {"lawsuit", 4}};
    for (const Ticket& t : inbox) {
        std::cout << "ticket '" << t.topic << "' (severity " << t.severity << ")\n";
        chain->handle(t);  // the client talks to the head of the chain only
    }
}
