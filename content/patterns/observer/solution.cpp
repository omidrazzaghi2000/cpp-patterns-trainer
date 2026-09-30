// Solution: a Shipment notifies every subscribed channel whenever its status changes.
#include <iostream>
#include <string_view>
#include <vector>

// Observer interface.
class ShipmentListener {
public:
    virtual ~ShipmentListener() = default;
    virtual std::string_view channel() const = 0;
    virtual void onStatus(std::string_view status) = 0;
};

class SmsAlert : public ShipmentListener {
public:
    std::string_view channel() const override { return "sms"; }
    void onStatus(std::string_view status) override {
        std::cout << "  sms: your parcel is " << status << '\n';
    }
};

class EmailAlert : public ShipmentListener {
public:
    std::string_view channel() const override { return "email"; }
    void onStatus(std::string_view status) override {
        std::cout << "  email: status changed to \"" << status << "\"\n";
    }
};

class TrackingPage : public ShipmentListener {
public:
    std::string_view channel() const override { return "web"; }
    void onStatus(std::string_view status) override {
        ++events_;
        std::cout << "  web: timeline event #" << events_ << " (" << status << ")\n";
    }
private:
    int events_ = 0;
};

// Subject: knows its listeners only through the ShipmentListener interface.
class Shipment {
public:
    void subscribe(ShipmentListener& listener) {
        listeners_.push_back(&listener);
        std::cout << listener.channel() << " subscribed\n";
    }
    void unsubscribe(ShipmentListener& listener) {
        std::erase(listeners_, &listener);
        std::cout << listener.channel() << " unsubscribed\n";
    }
    void setStatus(std::string_view status) {
        std::cout << "status: " << status << '\n';
        for (ShipmentListener* listener : listeners_) listener->onStatus(status);
    }

private:
    std::vector<ShipmentListener*> listeners_;  // non-owning
};

int main() {
    SmsAlert sms;
    EmailAlert email;
    TrackingPage web;
    Shipment parcel;

    parcel.subscribe(sms);
    parcel.subscribe(email);
    parcel.setStatus("shipped");

    parcel.subscribe(web);
    parcel.unsubscribe(sms);  // the customer muted text messages
    parcel.setStatus("out for delivery");
    parcel.setStatus("delivered");
}
