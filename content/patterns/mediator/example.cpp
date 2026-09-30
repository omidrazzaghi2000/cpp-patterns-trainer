// Mediator: a sign-up dialog coordinates its widgets so they never talk to each other.
#include <iostream>
#include <string>

// Mediator interface: the only thing a widget can say is "something happened to me".
class Widget;
class Dialog {
public:
    virtual ~Dialog() = default;
    virtual void widgetChanged(const Widget& sender) = 0;
};

// Colleague base class: knows its dialog, but not a single sibling widget.
class Widget {
public:
    explicit Widget(Dialog& dialog) : dialog_(dialog) {}
    Widget(const Widget&) = delete;  // a widget belongs to exactly one dialog
    virtual ~Widget() = default;
protected:
    void changed() const { dialog_.widgetChanged(*this); }
private:
    Dialog& dialog_;
};

class TextField : public Widget {
public:
    using Widget::Widget;
    void type(const std::string& text) {
        text_ = text;
        std::cout << "[email] typed \"" << text_ << "\"\n";
        changed();
    }
    const std::string& text() const { return text_; }
private:
    std::string text_;
};

class CheckBox : public Widget {
public:
    using Widget::Widget;
    void toggle() {
        checked_ = !checked_;
        std::cout << "[terms] " << (checked_ ? "checked" : "unchecked") << '\n';
        changed();
    }
    bool checked() const { return checked_; }
private:
    bool checked_ = false;
};

class Button : public Widget {
public:
    using Widget::Widget;
    void setEnabled(bool on) { enabled_ = on; }
    void click() const {
        std::cout << "[submit] clicked" << (enabled_ ? "" : " (disabled, ignored)") << '\n';
        if (enabled_) changed();
    }
private:
    bool enabled_ = false;
};

// Concrete mediator: owns the widgets and keeps ALL interaction rules in one place.
class SignUpDialog : public Dialog {
public:
    TextField email{*this};
    CheckBox terms{*this};
    Button submit{*this};

    void widgetChanged(const Widget& sender) override {
        if (&sender == &submit) {
            std::cout << "  dialog: account created for " << email.text() << '\n';
            return;
        }
        const bool validEmail = email.text().find('@') != std::string::npos;
        submit.setEnabled(validEmail && terms.checked());
        if (!validEmail)           std::cout << "  dialog: hint -> enter a valid email\n";
        else if (!terms.checked()) std::cout << "  dialog: hint -> accept the terms\n";
        else                       std::cout << "  dialog: submit enabled\n";
    }
};

int main() {
    SignUpDialog dialog;
    dialog.email.type("sara");
    dialog.email.type("sara@mail.com");
    dialog.submit.click();
    dialog.terms.toggle();
    dialog.submit.click();
}
