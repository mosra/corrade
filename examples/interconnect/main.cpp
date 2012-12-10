/*
    Copyright © 2007, 2008, 2009, 2010, 2011, 2012
              Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Corrade.

    Corrade is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Corrade is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include <string>
#include <Interconnect/Emitter.h>
#include <Interconnect/Receiver.h>
#include <Utility/Debug.h>

using namespace Corrade;

class RemoteControl: public Interconnect::Emitter {
    public:
        Signal triggered(const std::string& password, int timeout) {
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

    delete this; // commit suicide
}

int main(int, char**) {
    RemoteControl rc;
    Bomb *bomb1 = new Bomb,
         *bomb2 = new Bomb,
         *bomb3 = new Bomb;

    Interconnect::Emitter::connect(&rc, &RemoteControl::triggered, bomb1, &Bomb::launch);
    Interconnect::Emitter::connect(&rc, &RemoteControl::triggered, bomb2, &Bomb::launch);
    Interconnect::Emitter::connect(&rc, &RemoteControl::triggered, bomb3, &Bomb::launch);

    Utility::Debug() << "Successfully installed" << rc.signalConnectionCount() << "bombs.";

    rc.triggered("terrorist69", 60); // Launch all connected bombs after 60 seconds

    if(rc.signalConnectionCount()) {
        Utility::Error() << "Mission failed!" << rc.signalConnectionCount() << "bombs didn't explode!";
        return 1;
    }

    Utility::Debug() << "Mission succeeded!";
    return 0;
}
