// Exercise: draw the settings screen from ONE widget factory so themes never get mixed.
#include <iostream>
#include <memory>
#include <string>

// Abstract products
class Button {
public:
    virtual ~Button() = default;
    virtual std::string render(const std::string& label) const = 0;
};

class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual std::string render(const std::string& label, bool checked) const = 0;
};

// Light theme family
class LightButton : public Button {
public:
    std::string render(const std::string& label) const override {
        return "[ " + label + " ]";
    }
};

class LightCheckbox : public Checkbox {
public:
    std::string render(const std::string& label, bool checked) const override {
        return std::string(checked ? "[x] " : "[ ] ") + label;
    }
};

// Dark theme family
class DarkButton : public Button {
public:
    std::string render(const std::string& label) const override {
        return "<< " + label + " >>";
    }
};

class DarkCheckbox : public Checkbox {
public:
    std::string render(const std::string& label, bool checked) const override {
        return std::string(checked ? "(*) " : "( ) ") + label;
    }
};

// Abstract factory
class WidgetFactory {
public:
    virtual ~WidgetFactory() = default;
    virtual std::unique_ptr<Button> createButton() const = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() const = 0;
};

class LightFactory : public WidgetFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<LightButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<LightCheckbox>();
    }
};

// TODO 1: write class DarkFactory : public WidgetFactory
//         that creates a DarkButton and a DarkCheckbox.

// TODO 2: take a `const WidgetFactory&` parameter and create BOTH widgets
//         through it. Right now the concrete classes are hard-coded - and mixed!
void drawSettings() {
    const auto checkbox = std::make_unique<DarkCheckbox>();
    const auto button = std::make_unique<LightButton>();
    std::cout << checkbox->render("Email alerts", true) << '\n';
    std::cout << checkbox->render("Auto-update", false) << '\n';
    std::cout << button->render("Save") << '\n';
}

int main() {
    std::cout << "-- light --\n";
    drawSettings();  // TODO 3: pass a LightFactory
    std::cout << "-- dark --\n";
    drawSettings();  // TODO 3: pass a DarkFactory
}
