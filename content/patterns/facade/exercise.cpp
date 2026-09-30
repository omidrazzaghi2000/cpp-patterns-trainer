// Exercise: give a meeting room a Facade so starting a call is one method.
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
        // TODO 1: dim the lights to 40%, power the display on and select input "HDMI 1",
        //         wake the camera, and finally join the call `meetingId`.
    }

    void endMeeting() {
        std::cout << "room: ending\n";
        // TODO 2: undo it in reverse order: leave the call, put the camera to sleep,
        //         put the display in standby, and set the lights back to 100%.
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
