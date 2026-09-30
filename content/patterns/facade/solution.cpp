// Solution: MeetingRoom hides the right order of four devices behind two calls.
#include <iostream>
#include <string>
#include <string_view>

// ---- Subsystem classes (do not change them) ----
class Lights {
public:
    void setLevel(int percent) { std::cout << "lights: " << percent << "%\n"; }
};

class Display {
public:
    void powerOn() { on_ = true; std::cout << "display: on\n"; }
    void standby() { on_ = false; std::cout << "display: standby\n"; }
    void selectInput(std::string_view input) {
        if (!on_) { std::cout << "display: error, powered off\n"; return; }
        std::cout << "display: input " << input << '\n';
    }

private:
    bool on_ = false;
};

class Camera {
public:
    void wake() { std::cout << "camera: on\n"; }
    void sleep() { std::cout << "camera: off\n"; }
};

class VideoCall {
public:
    void join(std::string_view id) { id_ = id; std::cout << "call: joined " << id_ << '\n'; }
    void leave() { std::cout << "call: left " << id_ << '\n'; }

private:
    std::string id_;
};

// ---- Facade ----
class MeetingRoom {
public:
    void startMeeting(std::string_view meetingId) {
        std::cout << "room: starting " << meetingId << '\n';
        lights_.setLevel(40);
        display_.powerOn();
        display_.selectInput("HDMI 1");  // only valid once the display is on
        camera_.wake();
        call_.join(meetingId);
    }

    void endMeeting() {
        std::cout << "room: ending\n";
        call_.leave();
        camera_.sleep();
        display_.standby();
        lights_.setLevel(100);
    }

private:
    Lights lights_;
    Display display_;
    Camera camera_;
    VideoCall call_;
};

int main() {
    MeetingRoom room;  // the client only ever talks to the facade
    room.startMeeting("842-117");
    room.endMeeting();
}
