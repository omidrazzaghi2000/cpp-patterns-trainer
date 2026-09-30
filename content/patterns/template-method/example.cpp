// Template Method: CI jobs share one fixed step order; each project fills in the details.
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class CiJob {
public:
    virtual ~CiJob() = default;

    // The template method: public and NON-virtual (NVI idiom). Subclasses cannot
    // reorder, skip or forget a step - they can only customize the steps.
    void run() const {
        std::cout << "[ci] " << name() << '\n';
        std::cout << "  checkout sources\n";                 // invariant step
        std::cout << "  build: " << buildCommand() << '\n';  // primitive operation
        if (hasTests()) {                                     // hook
            std::cout << "  test: " << testCommand() << '\n';
        } else {
            std::cout << "  test: skipped\n";
        }
        std::cout << "  publish: " << artifact() << '\n';
    }

private:
    // Primitive operations: every job MUST provide them.
    virtual std::string name() const = 0;
    virtual std::string buildCommand() const = 0;
    virtual std::string artifact() const = 0;

    // Hooks: sensible defaults that a job MAY override.
    virtual bool hasTests() const { return true; }
    virtual std::string testCommand() const { return "ctest --output-on-failure"; }
};

// Overriding a private virtual is legal: a subclass supplies the step,
// but only CiJob::run() decides when it is called.
class GameEngineJob final : public CiJob {
    std::string name() const override { return "game-engine (C++)"; }
    std::string buildCommand() const override { return "cmake --build build"; }
    std::string artifact() const override { return "engine-2.1.tar.gz"; }
};

class WebShopJob final : public CiJob {
    std::string name() const override { return "web-shop (TypeScript)"; }
    std::string buildCommand() const override { return "npm run build"; }
    std::string testCommand() const override { return "npm test"; }
    std::string artifact() const override { return "shop-dist.zip"; }
};

class DocsJob final : public CiJob {
    std::string name() const override { return "user-docs"; }
    std::string buildCommand() const override { return "mkdocs build"; }
    bool hasTests() const override { return false; }  // plain text: nothing to test
    std::string artifact() const override { return "site.zip"; }
};

int main() {
    std::vector<std::unique_ptr<CiJob>> pipeline;
    pipeline.push_back(std::make_unique<GameEngineJob>());
    pipeline.push_back(std::make_unique<WebShopJob>());
    pipeline.push_back(std::make_unique<DocsJob>());

    for (const auto& job : pipeline) {
        job->run();  // same skeleton every time, different details
    }
}
