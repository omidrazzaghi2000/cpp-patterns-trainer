// Bridge: notification kinds and delivery channels vary independently.
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

// Implementation side: HOW a message physically leaves the system.
class Channel {
public:
    virtual ~Channel() = default;
    virtual void deliver(std::string_view to, std::string_view title,
                         std::string_view body) const = 0;
};

class EmailChannel : public Channel {
public:
    void deliver(std::string_view to, std::string_view title,
                 std::string_view body) const override {
        std::cout << "[email] " << to << ": " << title << " / " << body << '\n';
    }
};

class SmsChannel : public Channel {
public:
    void deliver(std::string_view to, std::string_view title,
                 std::string_view /*body*/) const override {
        std::cout << "[sms] " << to << ": " << title << '\n';  // SMS is short: title only
    }
};

// Abstraction side: WHAT we tell the user. It only knows the Channel interface.
class Notification {
public:
    explicit Notification(std::unique_ptr<Channel> channel) : channel_(std::move(channel)) {}
    virtual ~Notification() = default;
    virtual void send(std::string_view user) const = 0;

    // The bridge can be re-pointed at runtime without touching any subclass.
    void setChannel(std::unique_ptr<Channel> channel) { channel_ = std::move(channel); }

protected:
    const Channel& channel() const { return *channel_; }

private:
    std::unique_ptr<Channel> channel_;  // the bridge to the implementation hierarchy
};

class OrderShipped : public Notification {
public:
    OrderShipped(std::unique_ptr<Channel> channel, int orderId)
        : Notification(std::move(channel)), orderId_(orderId) {}

    void send(std::string_view user) const override {
        channel().deliver(user, "Order #" + std::to_string(orderId_) + " shipped",
                          "track it in the app");
    }

private:
    int orderId_;
};

class SecurityAlert : public Notification {
public:
    SecurityAlert(std::unique_ptr<Channel> channel, std::string device)
        : Notification(std::move(channel)), device_(std::move(device)) {}

    void send(std::string_view user) const override {
        channel().deliver(user, "New login: " + device_, "not you? reset password");
    }

private:
    std::string device_;
};

int main() {
    // Any kind works with any channel: M kinds + N channels, not M x N subclasses.
    OrderShipped shipped(std::make_unique<EmailChannel>(), 1042);
    SecurityAlert alert(std::make_unique<SmsChannel>(), "Firefox");
    shipped.send("sara@mail.com");
    alert.send("+1-555-0101");

    std::cout << "-- users changed their preferences --\n";
    shipped.setChannel(std::make_unique<SmsChannel>());
    alert.setChannel(std::make_unique<EmailChannel>());
    shipped.send("+1-555-0101");
    alert.send("sara@mail.com");
}
