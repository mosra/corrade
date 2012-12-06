#include <string>
#include <Interconnect/Emitter.h>
#include <Interconnect/Receiver.h>
#include <Utility/Debug.h>

using namespace Corrade;

class RemoteControl: public Interconnect::Emitter {
    public:
        Signal triggered(const std::string& password, int timeout) const {
            return emit(&RemoteControl::triggered, password, timeout);
        }
};

class Bomb: public Interconnect::Receiver {
    public:
        void launch(const std::string& password, int timeout);
};

void Bomb::launch(const std::string& password, int timeout) {
    if(password != "terrorist69") {
        Utility::Error() << "Wrong password. No apocalypse will be performed.";
        return;
    }

    Utility::Warning() << "Launching bomb in" << timeout << "seconds.";

    // ...
}

int main(int, char**) {
    RemoteControl rc;
    Bomb bomb1, bomb2, bomb3;

    Interconnect::Emitter::connect(&rc, &RemoteControl::triggered, &bomb1, &Bomb::launch);
    Interconnect::Emitter::connect(&rc, &RemoteControl::triggered, &bomb2, &Bomb::launch);
    Interconnect::Emitter::connect(&rc, &RemoteControl::triggered, &bomb3, &Bomb::launch);

    Utility::Debug() << "Successfully installed" << rc.connectionCount() << "bombs.";

    rc.triggered("terrorist69", 60); // Launch all connected bombs after 60 seconds

    return 0;
}
